package dsa.template

import common.MacroVar.{NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}
import op.{OPC, OpInfo}
import spec.{GibSpec, GpeSpec, LsuSpec}

import scala.collection.mutable
import scala.collection.mutable.ListBuffer

// GPE parameters
class GpeParam(spec : GpeSpec) {
  var num_rf_reg  = spec.num_rf_reg  // do not support heterogeneous parameter
  var max_delay  = spec.max_delay
  var operations  = spec.operations
  var from_dir = spec.from_dir
  var to_dir = spec.to_dir
  var num_input_per_operand  = ListBuffer.fill(operations.map(OPC.withName(_)).map(OpInfo.getOperandNum(_)).max){from_dir.size}

  def == (gpe : GpeParam): Boolean = {
    operations == gpe.operations &&  num_input_per_operand == gpe.num_input_per_operand
  }
  def update() : Unit ={
    num_input_per_operand = ListBuffer.fill(operations.map(OPC.withName(_)).map(OpInfo.getOperandNum(_)).max){from_dir.size}
  }
}

object GpeParam {
  def apply(spec : GpeSpec)= {
    new GpeParam(spec)
  }
}

// LSU parameters
class LsuParam(spec : LsuSpec) {
  var max_delay  = spec.max_delay
  var num_Perin  = spec.num_Perin
  var numOutmem  = spec.numOutmem
  var mem_depth  = spec.mem_depth

  def ==(lsu: LsuParam): Boolean = {
    max_delay == lsu.max_delay && num_Perin == lsu.num_Perin
  }
}

object LsuParam {
  def apply(spec : LsuSpec)= {
    new LsuParam(spec)
  }
}

// GIB parameters
class GibParam(spec : GibSpec){
  var pin2pin = spec.pin2pin
  var pin2track = spec.pin2track
  var track2pin = spec.track2pin
  require(pin2pin.size == 3)
  require(pin2track.size == 4)
  require(track2pin.size == 4)
  /*var connect_flexibility = mutable.Map(
    ("num_itrack_per_ipin" -> fclist(0)),
    ("num_otrack_per_opin" -> fclist(1)),
    ("num_ipin_per_opin" -> fclist(2))
  )*/
  var num_iopin_list = mutable.Map[String, Int]()
  var track_directions : ListBuffer[Int] = ListBuffer()
  var track_reged = false

  def == (gib : GibParam):Boolean = {
    pin2pin == gib.pin2pin &&
      pin2track == gib.pin2track &&
      track2pin == gib.track2pin &&
      num_iopin_list == gib.num_iopin_list
  }

  /*def update() : Unit ={
    connect_flexibility = mutable.Map(
      ("num_itrack_per_ipin" -> fclist(0)),
      ("num_otrack_per_opin" -> fclist(1)),
      ("num_ipin_per_opin" -> fclist(2))
    )
  }*/
}

object GibParam {
  def apply (spec : GibSpec) : GibParam = {
    new GibParam(spec)
  }
}

// CGRA Parameters
class CgraParam(attrs: mutable.Map[String, Any]){
  // ====== Global attributes =======//
  val rows = attrs("num_row").asInstanceOf[Int]     // PE number in a row
  val cols = attrs("num_colum").asInstanceOf[Int]   // PE number in a colum
  val dataWidth = attrs("data_width").asInstanceOf[Int] // data width in bit
  // cfgParams
  val cfgDataWidth = attrs("cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("cfg_addr_width").asInstanceOf[Int]
  val cfgBlkOffset = attrs("cfg_blk_offset").asInstanceOf[Int]   // configuration offset bit of blocks
  val maxII = attrs("max_II").asInstanceOf[Int] // configmem depth, maximum II supported

  // ====== GPE-Specific attributes =======//
  // parameters of GPEs in 2D
  val gpesSpec = attrs("gpes").asInstanceOf[ListBuffer[ListBuffer[GpeSpec]]]
  val gpesParam = gpesSpec.map{ buf =>
    buf.map{ spec => GpeParam(spec) }
  }
  // different types of GPEs (as submodules)
  // the type of each GPE (as instance)
  val gpe_typemap : mutable.Map[Int, GpeParam] =  mutable.Map() // [type-id, GpeParam]
  val gpe_posmap : mutable.Map[Tuple2[Int,Int],Int] = mutable.Map() // [(x, y), type-id]

  // ====== GIB-Specific attributes =======//
  val numTrack = attrs("num_track").asInstanceOf[Int]
  // trackRegedMode, 0: no reg; 1: half of GIBs reged; 2: all GIBs reged
  val trackRegedMode = attrs("track_reged_mode").asInstanceOf[Int]
  // parameters of GIBs in 2D
  val gibsSpec = attrs("gibs").asInstanceOf[ListBuffer[ListBuffer[GibSpec]]]
  val gibsParam = gibsSpec.map { buf =>
    buf.map { spec => GibParam(spec) }
  }
  // different types of GIBs (as submodules)
  // the type of each GIB (as instance)
  val gib_typemap: mutable.Map[Int, GibParam] = mutable.Map()
  val gib_posmap: mutable.Map[Tuple2[Int, Int], Int] = mutable.Map()
  // fcList
  //val fcMap = attrs("connect_flexibility").asInstanceOf[mutable.Map[String, Int]]
  // val fci  = fcMap("num_itrack_per_ipin")     // ipin-itrack connection flexibility, connected track number
  // val fco  = fcMap("num_otrack_per_opin")     // opin-otrack connection flexibility, connected track number
  // val fcio = fcMap("num_ipin_per_opin")       // opin-ipin  connection flexibility, connected ipin number
  // for ultra flexible connectivity
  //val pin2pin = attrs("pin_2_pin_connection").asInstanceOf[mutable.Map[String, Boolean]]
  //val pin2track = attrs("pin_2_track_connection").asInstanceOf[mutable.Map[String, Boolean]]


  // ====== IOB-Specific attributes =======//
  val numInOB = attrs("num_input_ob").asInstanceOf[Int]
  val numOutIB = attrs("num_output_ib").asInstanceOf[Int] // should be even
  val numOutOB = attrs("num_output_ob").asInstanceOf[Int]
  val numInIB = attrs("num_input_ib").asInstanceOf[Int]
  val numIn = 2*cols*numInIB
  val numOut = 2*cols*numOutOB


  // ====== LSU-Specific attributes =======//
  //val num_Perin = attrs("num_mem_perin").asInstanceOf[Int]
  val numOutmem = attrs("num_mem_out").asInstanceOf[Int]
  val mem_depth = attrs("memory_depth").asInstanceOf[Int]
  // parameters of LSUs in 2D
  val lsusSpec = attrs("lsus").asInstanceOf[ListBuffer[ListBuffer[LsuSpec]]]
  val lsusParam = lsusSpec.map { buf =>
    buf.map { spec => LsuParam(spec) }
  }
  // different types of LSUs (as submodules)
  // the type of each LSU (as instance)
  val lsu_typemap: mutable.Map[Int, LsuParam] = mutable.Map() // [type-id, GpeParam]
  val lsu_posmap: mutable.Map[Tuple2[Int, Int], Int] = mutable.Map() // [(x, y), type-id]

  // find different types of GPEs (as submodules) according to the GPE Parameter
  // get the type of each GPE (as instance)
  for( x <- 0 until gpesParam.size){
    for( y <- 0 until gpesParam(0).size){
      val gpe = gpesParam(x)(y)
      val res = gpe_typemap.find(ins => ins._2 == gpe)
      val type_id = {
        if(res != None){ // find a type
          res.get._1
        }else{ // create a new type
          val new_type_id = gpe_typemap.size
          gpe_typemap += (new_type_id -> gpe)
          new_type_id
        }
      }
      gpe_posmap += ((x,y) -> type_id)
    }
  }

  // find different types of LSUs (as submodules) according to the LSU Parameter
  // get the type of each LSU (as instance)
  for (x <- 0 until lsusParam.size) {
    for (y <- 0 until lsusParam(0).size) {
      val lsu = lsusParam(x)(y)
      val res = lsu_typemap.find(ins => ins._2 == lsu)
      val type_id = {
        if (res != None) { // find a type
          res.get._1
        } else { // create a new type
          val new_type_id = lsu_typemap.size
          lsu_typemap += (new_type_id -> lsu)
          new_type_id
        }
      }
      lsu_posmap += ((x, y) -> type_id)
    }
  }


  // find different types of GIBs (as submodules) according to the GIB Parameter
  // get the type of each GIB (as instance)
  //println("gibsParam.size: " + gibsParam.size)
  //println("gibsParam(0).size: " + gibsParam(0).size)
  for (i <- 0 until gibsParam.size) {
    for (j <- 0 until gibsParam(0).size) {
      val gib = gibsParam(i)(j)
      val num_iopin_list = mutable.Map[String, Int]()
      num_iopin_list += "ipin_nw" -> {
        if (i == 0 && j > 0) numInOB / 2
        else if (i > 0 && j > 0) {
          if (gpesParam(i - 1)(j - 1).from_dir.contains(SOUTHEAST)) {
            gpesParam(i - 1)(j - 1).num_input_per_operand.size // operand number
          } else {
            0
          }
        }
        else if (i > 0 && j == 0) lsusParam(i - 1)(j).num_Perin //new
        //else if(i == rows && j == 0) num_Perin                //new
        else 0
      }
      num_iopin_list += "opin_nw" -> {
        if (i == 0 && j > 0) numOutIB
        else if (i > 0 && j > 0) {
          if (gpesParam(i - 1)(j - 1).to_dir.contains(SOUTHEAST)) {
            1
          } else {
            0
          }
        }
        else if (i > 0 && j == 0) lsusParam(i - 1)(j).numOutmem //new
        //else if(i == rows && j == 0) numOutmem                //new
        else 0
      }
      num_iopin_list += "ipin_ne" -> {
        if (i == 0 && j < cols) numInOB / 2
        else if (i > 0 && j < cols) {
          if (gpesParam(i - 1)(j).from_dir.contains(SOUTHWEST)) {
            gpesParam(i - 1)(j).num_input_per_operand.size
          } else {
            0
          }
        }
        //else if((i == rows && j == cols)||(i > 0 && j == cols)) num_Perin                //new
        else if (i > 0 && j == cols) lsusParam(i - 1)(1).num_Perin //new
        else 0
      }
      num_iopin_list += "opin_ne" -> {
        if (i == 0 && j < cols) numOutIB
        else if (i > 0 && j < cols) {
          if (gpesParam(i - 1)(j).to_dir.contains(SOUTHWEST)) {
            1
          } else {
            0
          }
        }
        //else if((i == rows && j == cols)||(i > 0 && j == cols)) numOutmem                //new
        else if (i > 0 && j == cols) lsusParam(i - 1)(1).numOutmem //new
        else 0
      }
      num_iopin_list += "ipin_se" -> {
        if (i == rows && j < cols) numInOB / 2
        else if (i < rows && j < cols) {
          if (gpesParam(i)(j).from_dir.contains(NORTHWEST)) {
            gpesParam(i)(j).num_input_per_operand.size
          } else {
            0
          }
        }
        //else if((i == 0 && j == cols)||(i > 0 && j == cols)) num_Perin                //new
        //else if(i >= 0 && j == cols && i != rows) num_Perin                //new
        else if (i < rows && j == cols) lsusParam(i)(1).num_Perin //new
        else 0
      }
      num_iopin_list += "opin_se" -> {
        if (i == rows && j < cols) numOutIB
        else if (i < rows && j < cols) {
          if (gpesParam(i)(j).to_dir.contains(NORTHWEST)) {
            1
          } else {
            0
          }
        }
        //else if((i == 0 && j == cols)||(i > 0 && j == cols)) numOutmem               //new
        else if (i < rows && j == cols) lsusParam(i)(1).numOutmem //new
        else 0
      }
      num_iopin_list += "ipin_sw" -> {
        if (i == rows && j > 0) numInOB / 2
        else if (i < rows && j > 0) {
          if (gpesParam(i)(j - 1).from_dir.contains(NORTHEAST)) {
            gpesParam(i)(j - 1).num_input_per_operand.size
          } else {
            0
          }
        }
        else if (i < rows && j == 0) lsusParam(i)(j).num_Perin //new
        //else if(i > 0 && j == 0) num_Perin                //new
        else 0
      }
      num_iopin_list += "opin_sw" -> {
        if (i == rows && j > 0) numOutIB
        else if (i < rows && j > 0) {
          if (gpesParam(i)(j - 1).to_dir.contains(NORTHEAST)) {
            1
          } else {
            0
          }
        }
        else if (i < rows && j == 0) lsusParam(i)(j).numOutmem //new
        //else if(i > 0 && j == 0) numOutmem                //new
        else 0
      }
      gib.num_iopin_list = num_iopin_list
      val reged = {
        if (trackRegedMode == 0) false
        else if (trackRegedMode == 2) true
        else (i % 2 + j % 2) == 1
      }
      gib.track_reged = reged
      // which side has tracks
      val trackdirbuf: ListBuffer[Int] = ListBuffer()
      if (j == 0) trackdirbuf.append(0) else trackdirbuf.append(1) // WEST
      if (i == 0) trackdirbuf.append(0) else trackdirbuf.append(1) // NORTH
      if ((j + 1) == gibsParam(0).size) trackdirbuf.append(0) else trackdirbuf.append(1) // EAST
      if ((i + 1) == gibsParam.size) trackdirbuf.append(0) else trackdirbuf.append(1) // SOUTH
      gib.track_directions = trackdirbuf
      // find the type of each GIB
      val res = gib_typemap.find(ins => ins._2 == gib)
      val type_id = {
        if (res != None) { // find a type
          res.get._1
        } else { // create a new type
          val new_type_id = gib_typemap.size
          gib_typemap += (new_type_id -> gib)
          new_type_id
        }
      }
      gib_posmap += ((i, j) -> type_id)
    }
  }

}

object CgraParam{
  def apply(attrs: mutable.Map[String, Any]) = {
    new CgraParam(attrs)
  }
}