package dsa

import chisel3._
import chisel3.util.{log2Ceil, _}
import Chisel.{Decoupled}

/** Single port SRAM
 * 
 * @param width   data I/O width
 * @param depth   SRAM depth
 */ 
class SinglePortSRAM(width: Int, depth: Int) extends Module {
  val io = IO(new Bundle{
    val en = Input(Bool())
    val we = Input(Bool())
    val addr = Input(UInt(log2Ceil(depth).W))
    val din  = Input(UInt(width.W))
    val dout = Output(UInt(width.W)) // Latency = 1
  })

  val mem = Mem(depth, UInt(width.W))
  val dout = RegInit(0.U(width.W))

    io.dout := dout
  
    when(io.en) {
      when(io.we) {
        mem(io.addr) := io.din
      }.otherwise {
        dout := mem(io.addr) 
      }
    }
}



/** Simple dual port SRAM
 * 
 * @param width   data I/O width
 * @param depth   SRAM depth
 */ 
class SimpleDualPortSRAM(width: Int, depth: Int) extends Module {
  val io = IO(new Bundle{
    val ena   = Input(Bool())
    val wea   = Input(Bool())
    val addra = Input(UInt(log2Ceil(depth).W))
    val dina  = Input(UInt(width.W))
    val enb   = Input(Bool())
    val addrb = Input(UInt(log2Ceil(depth).W))
    val doutb = Output(UInt(width.W)) // Latency = 1
  })

  val mem = Mem(depth, UInt(width.W))
  val doutb = RegInit(0.U(width.W))

    io.doutb := doutb
  
    when(io.ena && io.wea) {
      mem(io.addra) := io.dina
    }
    when(io.enb) {
      doutb := mem(io.addrb) 
    }
}



/** True dual port SRAM
 * 
 * @param width   data I/O width
 * @param depth   SRAM depth
 */ 
class TrueDualPortSRAM(width: Int, depth: Int) extends Module {
  val io = IO(new Bundle{
    val ena   = Input(Bool())
    val wea   = Input(Bool())
    val addra = Input(UInt(log2Ceil(depth).W))
    val dina  = Input(UInt(width.W))
    val douta = Output(UInt(width.W)) // Latency = 1
    val enb   = Input(Bool())
    val web   = Input(Bool())
    val addrb = Input(UInt(log2Ceil(depth).W))
    val dinb  = Input(UInt(width.W))
    val doutb = Output(UInt(width.W)) // Latency = 1
  })

  val mem = Mem(depth, UInt(width.W))
  val douta = RegInit(0.U(width.W))
  val doutb = RegInit(0.U(width.W))

    io.douta := douta
    io.doutb := doutb
    
    when(io.ena) {
      when(io.wea) {
        mem(io.addra) := io.dina
      }.otherwise {
        douta := mem(io.addra) 
      }
    }

    when(io.enb) {
      when(io.web) {
        mem(io.addrb) := io.dinb
      }.otherwise {
        doutb := mem(io.addrb) 
      }
    }
}

/** Synthesized single port SRAM
 *
 * @param data_width   data I/O width
 * @param depth   SRAM depth
 */
class single_port_ram(data_width:Int,depth:Int) extends Module {
  val io = IO(new Bundle {
    val enable = Input(Bool())
    val write_enable = Input(Bool())
    val addr = Input(UInt(log2Ceil(depth).W))
    val dataIn = Input(UInt(data_width.W))
    val dataOut = Output(UInt(data_width.W))
  })

  val mem = SyncReadMem(depth, UInt(data_width.W))
  io.dataOut := DontCare
  when(io.enable) {
    val rdwrPort = mem(io.addr)
    when (io.write_enable) { rdwrPort := io.dataIn }
      .otherwise    { io.dataOut := rdwrPort }
  }
}
/** CPU-LSU interface
 *
 * @param data_width   data I/O width
 * @param addr_width   addr depth
 * @param maxII   maximum supported II
 */
case class CPUInterface(addr_width:Int,data_width:Int,maxII:Int) extends Bundle{
  //val read_req = Input(Bool())
  //val read_ack = Output(Bool())
  val read_addr = Input(UInt(addr_width.W))
  val read_data = Decoupled(UInt(data_width.W))
  //val read_data = Output(UInt(data_width.W))

  //val write_req = Input(Bool())
  //val write_ack = Output(Bool())
  val write_addr = Input(UInt(addr_width.W))
  val write_data = Flipped(Decoupled(UInt(data_width.W)))
  //val write_data = Input(UInt(data_width.W))

  //indicate which t-block writes to
  val cycle = Input(UInt(log2Ceil(maxII).W))
}


/** Enable_delay modeule
 *
 * @param maxnum   maximum delay cycle
 */

class Delay_en(maxnum: Int) extends Module{
  val cfgWidth = log2Ceil(maxnum)+1
  val io = IO(new Bundle {
    val Config = Input(UInt(cfgWidth.W))
    val in_en = Input(Bool())
    val en_gate = Output(Bool())
  })
  val begin::inc::finish::Nil= Enum(3)
  val state = RegNext(begin)
  val delay = RegInit(0.U(cfgWidth.W))
  io.en_gate := DontCare
  when(io.Config === 0.U){
    io.en_gate := io.in_en
  }.otherwise{
    switch(state){
      is(begin){
        when(io.in_en){
          state := inc
          delay := 0.U
        }.otherwise{
          state := begin
        }
        io.en_gate := false.B
      }
      is(inc){
        when(delay === io.Config - 1.U) // one cycle earlier
        {
          io.en_gate := true.B
          state := finish
        }.otherwise{
          io.en_gate := false.B
          state := inc
          delay := delay +1.U
        }
      }
      is(finish){
        io.en_gate := true.B
        state := finish
      }
    }
  }
}

 object SRAMVerilogGen extends App {
   (new chisel3.stage.ChiselStage).emitVerilog(new single_port_ram(32, 64),args)
 }
