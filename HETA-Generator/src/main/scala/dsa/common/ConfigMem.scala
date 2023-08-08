package dsa

import chisel3._
import chisel3.util._

import scala.collection.mutable.ArrayBuffer


/** Configuration Memory (Registers)
 * 
 * @param regWidth        register data width
 * @param regNum          register number
 * @param cfgDataWidth    config data width
 * @param maxII           maximum II supported
 */ 
/*class ConfigMem(regWidth: Int, regNum: Int, cfgDataWidth: Int) extends Module {
  if(regNum > 1) {
    assert(cfgDataWidth % regWidth == 0)
  }
  val cfgAddrWidth = log2Ceil((regWidth * regNum + cfgDataWidth - 1) / cfgDataWidth)

  val io = IO(new Bundle {
    val cfg_en = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val out = Output(Vec(regNum, UInt(regWidth.W)))
  })

  // configuration registers
  val regs = RegInit(VecInit(Seq.fill(regNum){0.U(regWidth.W)}))

  if(cfgDataWidth >= regWidth){
    val step = cfgDataWidth / regWidth
    for(i <- 0 until regNum){
      when(io.cfg_en && (io.cfg_addr === (i/step).U)){
        regs(i) := io.cfg_data(((i%step)+1)*regWidth-1, (i%step)*regWidth)
      }   
    }
  } else{
    val num = (regWidth + cfgDataWidth - 1) / cfgDataWidth
    for(i <- 0 until num){
      when(io.cfg_en && (io.cfg_addr === i.U)){
        if(i == 0){
          regs(0) := Cat(regs(0)(regWidth-1, (i+1)*cfgDataWidth), io.cfg_data)
        }else if(i == num-1){
          regs(0) := Cat(io.cfg_data, regs(0)(cfgDataWidth-1, 0))
        }else{
          regs(0) := Cat(regs(0)(regWidth-1, (i+1)*cfgDataWidth), io.cfg_data, regs(0)(i*cfgDataWidth-1, 0))
        }
      }   
    }
  }
  
  // wire out all registers
  io.out.zipWithIndex.map{ case (y, x) => y := regs(x) }
}*/
class ConfigMem(regWidth: Int, regNum: Int, cfgDataWidth: Int, maxII: Int) extends Module {
  if(regNum > 1) {
    assert(cfgDataWidth % regWidth == 0)
  }
  // II width
  val II_Width = log2Ceil(maxII)

  val cfgAddrWidth = log2Ceil((regWidth * regNum + cfgDataWidth - 1) / cfgDataWidth)
  val cycleWidth = II_Width
  val io = IO(new Bundle {
    val cfg_en = Input(Bool())
    val en = Input(Bool())
    val cycle = Input(UInt(cycleWidth.W))//cycle within II
    val II = Input(UInt(cycleWidth.W))
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val out = Output(Vec(regNum, UInt(regWidth.W)))
    val current_t = Output(UInt(II_Width.W))
  })
  //println("cfgAddrWidth: " + cfgAddrWidth)
  // configuration registers
  //val regs = RegInit(VecInit(Seq.fill(regNum){0.U(regWidth.W)}))
  /*val regs : Vec[UInt]= if(cfgDataWidth >= regWidth){
    VecInit(Seq.fill(regNum){0.U(regWidth.W)}) //存储某一个reg一个cycle的配置
  }else{
    RegInit(VecInit(Seq.fill(regNum){0.U(regWidth.W)}))
  }*/
  val configreg = RegInit(VecInit(Seq.fill(maxII){0.U(regWidth.W)}))
  //val configreg = VecInit(Seq.fill(maxII){0.U(regWidth.W)}) //存储某一个reg所有cycle的配置
  //val configmem = Wire(Vec(regNum,chiselTypeOf(configreg)))
  val configmem = new ArrayBuffer[Vec[UInt]]()
  /*val configmem = new ArrayBuffer[SyncReadMem[UInt]]()
  for(i <- 0 until regNum){
    val conmem = SyncReadMem(maxII,UInt(regWidth.W))
    configmem += conmem
  }*/
  // Select the current configuration by Round-Robin
  // create pointer
  for(i <- 0 until maxII){
    configmem += configreg
  }
  val config_pointer_i =RegInit(0.U(II_Width.W))
  val config_pointer =RegNext(config_pointer_i)
  //regWidth是实际需要的配置位宽
  if(cfgDataWidth >= regWidth){
    val regs = VecInit(Seq.fill(regNum){0.U(regWidth.W)}) //存储某一个reg一个cycle的配置
    val step = cfgDataWidth / regWidth
    for(i <- 0 until regNum){
      when(io.cfg_en && (io.cfg_addr === (i/step).U)){
        regs(i) := io.cfg_data(((i%step)+1)*regWidth-1, (i%step)*regWidth)
        configmem(i)(io.cycle) := regs(i)
      }
    }
  } else{
    val num = (regWidth + cfgDataWidth - 1) / cfgDataWidth
   //println("num: " + num)
    for(i <- 0 until num){
      when(io.cfg_en && (io.cfg_addr === i.U)){
        if(i == 0){
          configmem(0)(io.cycle) := Cat( configmem(0)(io.cycle)(regWidth-1, (i+1)*cfgDataWidth), io.cfg_data)
        }else if(i == num-1){
          configmem(0)(io.cycle) := Cat(io.cfg_data,  configmem(0)(io.cycle)(i*cfgDataWidth-1, 0))
        }else{
          configmem(0)(io.cycle) := Cat( configmem(0)(io.cycle)(regWidth-1, (i+1)*cfgDataWidth), io.cfg_data,  configmem(0)(io.cycle)(i*cfgDataWidth-1, 0))
        }
      }
    }
  }
  //increase pointer
  when(io.en){
    config_pointer_i := Mux(config_pointer_i === io.II,0.U,config_pointer_i + 1.U)
  }.otherwise{
    config_pointer_i := 0.U
  }
  // output current time step, used by LSU
  io.current_t := config_pointer
  // wire out all registers
  io.out.zipWithIndex.map{ case (y, x) => y := configmem(x)(config_pointer) }
}

 object VerilogGen extends App {
   print(60)
   (new chisel3.stage.ChiselStage).emitVerilog(new ConfigMem(60, 1, 32, 8),args)
 }

