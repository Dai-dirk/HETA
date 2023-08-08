package ppa

import scala.collection.mutable
import scala.collection.mutable.{Map}
import ir._

object sm_area {
  val area_mux32_map : Map[Int, Int] = Map(
    1 -> 30,
    2 -> 78,
    3 -> 103,
    4 -> 152,
    5 -> 203,
    6 -> 231,
    7 -> 273
  )

  val area_delay32_map : mutable.Map[Int, Int] = mutable.Map(
    1 -> 575,
    2 -> 906,
    3 -> 1178,
    4 -> 1598,
    5 -> 1771,
    6 -> 2026,
    7 -> 2273
    )

  val area_alu32_map: mutable.Map[String, Int] = mutable.Map(
    "PASS" -> 51,
    "ADD" -> 234,
    "SUB" -> 243,
    "MUL" -> 2498,
    "AND" -> 59,
    "OR" -> 59,
    "XOR" -> 88,
    "SHL" -> 313,
    "LSHR" -> 313,
    "ASHR" -> 327,
    "EQ" -> 117, //
    "NE" -> 117,
    "LT" -> 145, //
    "LE" -> 146,
    "SEL" -> 70
  )

  val area_cfg32_map: mutable.Map[Int,Int] = mutable.Map(
    2 -> 18,
    3 -> 30,
    4 -> 40,
    5 -> 51,
    6 -> 60,
    7 -> 71,
    8 -> 80
  )

  var area_regnxt32 = 151 //暂时先忽略
  //  val area_rf32 = 338 //目前所有rf都是一个把
  var area_rf32 = 251

  //val area_cfgpre32 = 80
  var cfg_rate = 1.07

  var area_const32 = 0

  /*var cycle = 5
  var reduce_rate = 0.0*/

  var alu_rate = 1.115

  // for lsu
  var area_sram_32x64 = 17204 // the depth is 64

//  val ara_par : mutable.Map[String, Any] = mutable.Map(
//    "mux_area" -> area_mux32_map,
//    "alu_area" -> area_alu32_map,
//    "delay_area" -> area_delay32_map,
//    "reg_area" -> area_regnxt32,
//    "rf_area" -> 251,
//    "cfg_area" -> area_cfg32_map,
//    "cfg_rate" -> cfg_rate,
//    "const_area" -> 0,
//    "alu_rate" -> alu_rate,
//    "sram_area" -> 17204
//  )

  def loadSpec(jsonFile: String): Unit = {
    val jsonMap = IRHandler.loadIR(jsonFile)
    for (kv <- jsonMap) {
      if (kv._1 == "Muxn") {
        val mux_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        mux_map.foreach{ x =>
          area_mux32_map.put(x._1.toInt, math.ceil(x._2).toInt )
        }
        //println("area_mux32_map: " + area_mux32_map)
      } else if (kv._1 == "ALU") {
        val alu_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        alu_map.foreach { x =>
          area_alu32_map.put(x._1, math.ceil(x._2).toInt)
        }
        //println("area_alu32_map: " + area_alu32_map)
      } else if (kv._1 == "ConfigMem") {
        val cfg_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, List[List[Double]]]]
        cfg_map.foreach{ case(ii, area) =>
          area_cfg32_map.put(ii.toInt, math.ceil(area(0)(1)).toInt)
        }
       // println("area_cfg32_map: " + area_cfg32_map)
        //println("cfg_map: " + cfg_map)
      } else if (kv._1 == "SRAM") {
        val sram_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        area_sram_32x64 = math.ceil(sram_map("SRAMX32x64")).toInt
        //println("area_sram_32x64: " + area_sram_32x64)
      } else if (kv._1 == "RF") {
        val rf_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        area_rf32 =math.ceil(rf_map("RFX32X1X1X2")).toInt
        //println("area_rf32: " + area_rf32)
      } else if (kv._1 == "Reg") {
        val reg_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        area_regnxt32 =math.ceil(reg_map("RegX32")).toInt
        //println("area_regnxt32: " + area_regnxt32)
      } else if (kv._1 == "DelayPipe") {
        val delay_map = kv._2.asInstanceOf[scala.collection.immutable.Map[String, Double]]
        delay_map.foreach { x =>
          area_delay32_map.put(x._1.toInt, math.ceil(x._2).toInt)
        }
        //println("area_delay32_map: " + area_delay32_map)
      }
    }
  }

//  def loadSpec(jsonFile : String): Unit ={
//    val jsonMap = IRHandler.loadIR(jsonFile)
//    for(kv <- jsonMap){
//      if(kv._1 == "Muxn"){
//        ara_par("mux_area") = kv._2.asInstanceOf[Map[Int, Int]]
//      }else if(kv._1 == "ALU"){
//        ara_par("alu_area") = kv._2.asInstanceOf[Map[String, Int]]
//      }else if(kv._1 == "ConfigMem"){
//        ara_par("cfg_area") = kv._2.asInstanceOf[Map[Int, List[Int]]]
//      }else if (kv._1 == "SRAM") {
//        val area_sram_map = kv._2.asInstanceOf[Map[String, Int]]
//        ara_par("sram_area") = area_sram_map("SRAMX32x64")
//      }else if (kv._1 == "RF") {
//        val area_rf_map = kv._2.asInstanceOf[Map[String, Int]]
//        ara_par("rf_area") = area_rf_map("RFX32X1X1X2")
//      }else if (kv._1 == "Reg") {
//        ara_par("reg_area") = kv._2.asInstanceOf[Map[String, Int]]
//      } else if (kv._1 == "DelayPipe") {
//        ara_par("delay_area") = kv._2.asInstanceOf[Map[Int, Int]]
//      }
//    }
//  }
}
