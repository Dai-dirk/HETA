package dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable._
import op._
import scala.collection.mutable


/** reconfigurable arithmetic unit
 * 
 * @param width   data width
 */
class ALU(width: Int, ops: ListBuffer[OPC.OPC]) extends Module {
  val maxNumOperands = ops.map(OpInfo(width).getOperandNum(_)).max
  val cfgDataWidth = log2Ceil(OPC.numOPC)
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(Vec(maxNumOperands, UInt(width.W)))
    val out = Output(UInt(width.W)) 
  })

  val op2res = ops.map{op => 
    (op.id.U -> OpInfo.OpFuncMap(op)(io.in.toSeq))}

  io.out := MuxLookup(io.config, 0.U, op2res)
 }


object ALUVerilogGen extends App {
  //val mdl = new ALU(64, ListBuffer(OPC.ADD, OPC.SUB, OPC.SEL)).suggestName("my_test")
  val opmap: mutable.Map[String, OPC.OPC] = mutable.Map(
    "PASS" -> OPC.PASS,
    "ADD" -> OPC.ADD,
    "SUB" -> OPC.SUB,
    "MUL" -> OPC.MUL,
    "AND" -> OPC.AND,
    "OR" -> OPC.OR,
    "XOR" -> OPC.XOR,
    "SHL" -> OPC.SHL,
    "LSHR" -> OPC.LSHR,
    "ASHR" -> OPC.ASHR,
    "EQ" -> OPC.EQ,
    "NE" -> OPC.NE,
    "LT" -> OPC.LT,
    "LE" -> OPC.LE,
    "IMUL" -> OPC.IMUL,
    "ILE" -> OPC.ILE,
    "ILT" -> OPC.ILT,
    "SEL" -> OPC.SEL
  )
  val op = ListBuffer[OPC.OPC]()
  for (i <- 1 until args.size - 2) {
    op.append(opmap(args(i)))
  }
  (new chisel3.stage.ChiselStage).emitVerilog(new ALU(args(0).toInt, op), args)
}
