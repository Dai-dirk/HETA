package  dsa.LSU
import chisel3._

import scala.collection.mutable
import scala.collection.mutable.ListBuffer
import Chisel.log2Ceil
import dsa._
import ir._
/** IO Block
 *
 * @param attrs     module attributes
 */
class LSU (attrs: mutable.Map[String, Any]) extends Module with IR {
  apply(attrs)
  val width = getAttrValue("data_width").asInstanceOf[Int]
  val num_Perin = getAttrValue("num_mem_perin").asInstanceOf[Int]
  val numOut = getAttrValue("num_mem_out").asInstanceOf[Int]
  val mem_depth = getAttrValue("memory_depth").asInstanceOf[Int]
  // max delay cycles of the DelayPipe
  val maxDelay = getAttrValue("max_delay").asInstanceOf[Int]
  //val addr_width = log2Ceil(mem_depth)
  // cfgParams
  val cfgDataWidth = getAttrValue("cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = getAttrValue("cfg_addr_width").asInstanceOf[Int]
  val cfgBlkIndex = getAttrValue("cfg_blk_index").asInstanceOf[Int] // configuration index of this block, cfg_addr[width-1 : offset]
  val cfgBlkOffset = getAttrValue("cfg_blk_offset").asInstanceOf[Int] // configuration offset bit of blocks
  //@Yuan: configmem depth, maximum II supported
  val maxII = getAttrValue("max_II").asInstanceOf[Int]


  //the total memory depth = mem_dep_percycle * maxII
  //val all_mem_depth = mem_depth * maxII
  //println("all_mem_depth: " + all_mem_depth + " log2Ceil(all_mem_depth): "+log2Ceil(all_mem_depth))
  val io = IO(new Bundle {
    val cfg_en = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val hostInterface = CPUInterface(log2Ceil(mem_depth), width, maxII)
    val en = Input(Bool())
    val in = Input(Vec(num_Perin*2, UInt(width.W)))
    val out = Output(Vec(numOut, UInt(width.W)))
  })
  val write_data = Wire(UInt(width.W))
  val chip_enable = Wire(Bool())
  val write_enable = Wire(Bool())
  val addr = Wire(UInt(log2Ceil(mem_depth).W))
  val Oprand = Wire(Vec(2,UInt(width.W)))
  val opmode = Wire(UInt(cfgDataWidth.W))
  val const = Wire(UInt(width.W))
  // current cycle
  val current_t = Wire(UInt(log2Ceil(maxII).W))

  val muxs = Array.fill(2){Module(new Muxn(width, num_Perin+1)).io}
  val delay_pipes = Array.fill(2){ Module(new DelayPipe(width, maxDelay)).io }

  for(i <- 0 until 2){
      for(j <- 0 until num_Perin){
        muxs(i).in(j) := io.in(i+j*2)
      }
      muxs(i).in(num_Perin) := const
      delay_pipes(i).en := io.en
      delay_pipes(i).in := muxs(i).out
      Oprand(i) := delay_pipes(i).out
    }
  //("IIWidth : " + IIWidth)
  val mem_per_cycle = mem_depth / maxII
  // println("mem_per_cycle: " + mem_per_cycle)
  var cfg_base_addr = mem_per_cycle.U * current_t
  /*var cfg_base_addr = Wire(UInt(log2Ceil(all_mem_depth).W))
  cfg_base_addr :=  curCycle * mem_depth.U*/
  //println("mem_depth: " + mem_depth)
  //the base_addr that host writes to
  val host_base_addr = io.hostInterface.cycle * mem_per_cycle.U
  /*val host_base_addr = Wire(UInt(log2Ceil(all_mem_depth).W))
  host_base_addr := io.hostInterface.cycle * mem_depth.U*/
  //SRAM
  val mem = Module(new single_port_ram(width, mem_depth))
  mem.io.enable := chip_enable
  mem.io.write_enable := write_enable
  mem.io.addr := addr
  io.out(0) := mem.io.dataOut
  mem.io.dataIn := write_data

  // Host read data from LSU
  io.hostInterface.read_data.bits := mem.io.dataOut
  //val read_data_delay = RegNext(io.hostInterface.read_req)
  io.hostInterface.read_data.valid := io.hostInterface.read_data.ready

  // Host writes data to LSU
  io.hostInterface.write_data.ready := io.hostInterface.write_data.valid

  //using MUX to select input data to sram, according to the mode
  write_data := Mux(io.hostInterface.write_data.valid, io.hostInterface.write_data.bits, Oprand(0))

  when(io.hostInterface.read_data.ready) {
    chip_enable := true.B
    write_enable := false.B
    addr := io.hostInterface.read_addr(log2Ceil(mem_depth) - 1, 0) + host_base_addr
  }.elsewhen(io.hostInterface.write_data.valid) {
    chip_enable := true.B
    write_enable := true.B
    addr := io.hostInterface.write_addr(log2Ceil(mem_depth) - 1, 0) + host_base_addr
  }.otherwise {
    when(opmode === 1.U) {
      chip_enable :=  io.en
      write_enable := !io.en
      addr := Oprand(0)(log2Ceil(mem_depth) - 1, 0) + cfg_base_addr
    }.elsewhen(opmode === 2.U) {
      chip_enable := io.en
      write_enable := io.en
      addr := Oprand(1)(log2Ceil(mem_depth) - 1, 0) + cfg_base_addr
    }.otherwise {
      chip_enable := false.B
      write_enable := false.B
      addr := DontCare
    }
  }

  //configuration setting
  val constCfgWidth = width // constant
  val selWidth = log2Ceil(num_Perin+1)
  val delayCfgWidthEach = delay_pipes(0).config.getWidth // DelayPipe Config width
  val delayCfgWidth = 2 * delayCfgWidthEach
  val sumCfgWidth = 2 + constCfgWidth + delayCfgWidth + selWidth*2 // opcode-width + delay + sel + const
  //current cycle that cfg_data belongs to
  val IIWidth = log2Ceil(maxII)

  val cfg = Module(new ConfigMem(sumCfgWidth, 1, cfgDataWidth, maxII)) // configuration memory
  cfg.io.cfg_en := io.cfg_en && (cfgBlkIndex.U === io.cfg_addr(cfgAddrWidth-1, 2*IIWidth + cfgBlkOffset))
  cfg.io.cfg_addr := io.cfg_addr(2*IIWidth + cfgBlkOffset - 1, 2*IIWidth)
  cfg.io.en := io.en // input enable, begin to change config every cycle
  cfg.io.cycle := io.cfg_addr(2*IIWidth-1, IIWidth)// current cfg_data belongs to which cycle
  cfg.io.II := io.cfg_addr(IIWidth - 1, 0)//  II of current dfg
  cfg.io.cfg_data := io.cfg_data
  assert(cfg.cfgAddrWidth <= cfgBlkOffset)
  assert(cfgBlkIndex < (1 << (cfgAddrWidth-cfgBlkOffset)))

  // current time step
  current_t := cfg.io.current_t

  var offset = 0
  val cfgOut = Wire(UInt(sumCfgWidth.W))
  cfgOut := cfg.io.out(0)
  const := cfgOut(constCfgWidth-1, 0)
  offset = constCfgWidth

  for(i <- 0 until 2){
    muxs(i).config := cfgOut(selWidth+offset-1,offset)
    offset += selWidth
  }

  for(i <- 0 until 2){
    if(delayCfgWidthEach != 0){
      delay_pipes(i).config := cfgOut(offset+delayCfgWidthEach-1, offset)
    } else {
      delay_pipes(i).config := DontCare
    }
    offset += delayCfgWidthEach
  }

  opmode:= cfg.io.out(0)(sumCfgWidth-1,offset)


  // ======= sub_module attribute ========//

  val sm_id: Map[String, Int] = {
    Map(
      "SRAM" -> 1,
      "Const" -> 2,
      "DelayPipe" -> 3,
      "Muxn" -> 4
    )}
  val sub_modules = sm_id.map{case (name, id) => Map(
    "id" -> id,
    "type" -> name
  )}
  apply("sub_modules", sub_modules)

  // ======= sub_module instance attribute ========//
  // apply("sub_module_format", ("id", "type"))
  // 0 : this module
  // 1-n : sub-modules
  val smi_id: Map[String, List[Int]] = {
    Map(
      "This" -> List(0),
      "SRAM" -> List(1),
      "Const" -> List(2),
      "DelayPipe" -> (3 until 5).toList,
      "Muxn" -> (5 to 6).toList
    )
  }
  val instances = smi_id.map{case (name, ids) =>
    ids.map{id => Map(
      "id" -> id,
      "type" -> name,
      "module_id" -> {if(name == "This") 0 else sm_id(name)}
    )}
  }.flatten
  apply("instances", instances)
  // ======= connections attribute ========//
  // apply("connection_format", ("src_id", "src_type", "src_out_idx", "dst_id", "dst_type", "dst_in_idx"))
  // This:src_out_idx is the input index
  // This:dst_in_idx is the output index
  val connections = ListBuffer[(Int, String, Int, Int, String, Int)]()

    for(i <- 0 until 2*num_Perin){
        connections.append((smi_id("This")(0), "This", i, smi_id("Muxn")(i%2), "Muxn", i/2))
    }
    for(i <- 0 until 2 ){
        connections.append((smi_id("Const")(0), "Const", 0, smi_id("Muxn")(i), "Muxn", num_Perin))
        connections.append((smi_id("Muxn")(i), "Muxn", 0, smi_id("DelayPipe")(i), "DelayPipe", 0))
        connections.append((smi_id("DelayPipe")(i), "DelayPipe", 0, smi_id("SRAM")(0), "SRAM", i))
    }
    (0 until numOut).map{ i =>
      connections.append((smi_id("SRAM")(0), "SRAM", 0, smi_id("This")(0), "This", i))
    }
  // apply("connections", connections)
  apply("connections", connections.zipWithIndex.map{case (c, i) => i -> c}.toMap)
  // ======= configuration attribute ========//
  offset = 0;
    val configuration = mutable.Map( // id : type, high, low
      smi_id("This")(0) -> ("This", sumCfgWidth-1, 0),
      smi_id("Const")(0) -> ("Const", constCfgWidth-1, 0)
    )
    offset += constCfgWidth
    for(i <- 0 until 2){
      configuration += smi_id("Muxn")(i) -> ("Muxn", offset+selWidth-1, offset)
      offset += selWidth
    }
  for(i <- 0 until 2){
    configuration += smi_id("DelayPipe")(i) -> ("DelayPipe", offset+delayCfgWidthEach-1, offset)
    offset += delayCfgWidthEach
  }
    configuration += smi_id("SRAM")(0) -> ("SRAM", sumCfgWidth-1, offset)
    apply("configuration", configuration)
}
