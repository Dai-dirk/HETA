package mg

import spec._
import op._
import dsa._
import ppa.ppa_cgra.CGRA_area
//import ir._

// TODO: add to command options
//case class Config(
//  loadSpec: Boolean = true,
//  dumpOperations: Boolean = true,
//  dumpIR: Boolean = true,
//  genVerilog: Boolean = true,
//)

// CGRA Modeling and Generation
object CGRAMG extends App{
  var loadSpec : Boolean = true
  var dumpOperations : Boolean = false
  var dumpIR : Boolean = true
  var genVerilog : Boolean = false
  var getArea: Boolean = true

  if(loadSpec){
    val jsonFile = "src/main/resources/cgra_spec.json"
//    CGRASpec.dumpSpec(jsonFile)
    CGRASpec.loadSpec(jsonFile)
  }
  if(dumpOperations){
    val jsonFile = "src/main/resources/operations.json"
    OpInfo.dumpOpInfo(jsonFile)
  }
  if(genVerilog){
    (new chisel3.stage.ChiselStage).emitVerilog(new CGRA(CGRASpec.attrs, dumpIR), args)
  }else{ // not emit verilog to speedup
    (new chisel3.stage.ChiselStage).emitChirrtl(new CGRA(CGRASpec.attrs, dumpIR), args)
  }
  if (getArea) {
    val jsonFile = "src/main/resources/fastModel.json"
    val area = CGRA_area(CGRASpec.attrs, jsonFile)
  }
}
