package ppa

import chisel3._
import chisel3.util._
import common.MacroVar.{NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}
import dsa.template.{GibParam, GpeParam}
import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}

import java.io._

object ppa_cgra {
  def CGRA_area(attrs: mutable.Map[String, Any], jsonFile : String)  {
    sm_area.loadSpec(jsonFile)
//    println(sm_area.ara_par)
    // CGRA parameters
    val param = dsa.template.CgraParam(attrs)
    import param._

    var area : Double = 0
    var total_area : Double = 0
    val max_II = maxII
    for (k <- 0 until 2) { // top and buttom row
      for (i <- 0 until cols) {
        area = area + ppa.ppa_iob.getiobarea(1, numOutIB, max_II)
      }
    }
    total_area += area
    println("IB total area : " + area)

    area = 0
    for (k <- 0 until 2) { // top and buttom row
      for (i <- 0 until cols) {
        area = area + ppa.ppa_iob.getiobarea(numInOB, 1, max_II)
      }
    }
    total_area += area
    println("OB total area : " + area)

    area = 0
    for (i <- 0 until rows) {
      for (j <- 0 until cols) {
        val gpe_type = gpe_posmap((i, j))
        val gpe_param = gpe_typemap(gpe_type)
        area = area + ppa.ppa_gpe.getgpearea(gpe_param.operations, gpe_param.num_input_per_operand, gpe_param.max_delay, maxII)
      }
    }
    total_area += area
    println("GPE total area : " + area)

    area = 0
    for (i <- 0 to rows) {
      for (j <- 0 to cols) {
        val gib_type = gib_posmap(i, j)
        val gib_param = gib_typemap(gib_type)
        val track_reged = gibsParam(i)(j).track_reged // track_reged is not used to classify type
        val pin2pinMap = gib_param.pin2pin
        val p2p_side1 = pin2pinMap("num_ipin_side1") // ipin-itrack connection flexibility, connected track number, 2n
        val p2p_side2 = pin2pinMap("num_ipin_side2") // opin-otrack connection flexibility, connected track number, 2n
        val p2p_side3 = pin2pinMap("num_ipin_side3") // opin-ipin  connection flexibility, connected ipin number, 2n
        val pin2pinList = List(p2p_side1, p2p_side2, p2p_side3)
        val pin2trackMap = gib_param.pin2track
        val pin_2_track_side1 = pin2trackMap("num_otrack_side1")
        val pin_2_track_side2 = pin2trackMap("num_otrack_side2")
        val pin_2_track_side3 = pin2trackMap("num_otrack_side3")
        val pin_2_track_side4 = pin2trackMap("num_otrack_side4")
        val pin2trackList = List(pin_2_track_side1, pin_2_track_side2, pin_2_track_side3, pin_2_track_side4)
        val track2pinMap = gib_param.track2pin
        val track_2_pin_side1 = track2pinMap("num_itrack_side1")
        val track_2_pin_side2 = track2pinMap("num_itrack_side2")
        val track_2_pin_side3 = track2pinMap("num_itrack_side3")
        val track_2_pin_side4 = track2pinMap("num_itrack_side4")
        val track2pinList = List(track_2_pin_side1, track_2_pin_side2, track_2_pin_side3, track_2_pin_side4)
        //println("gib_param.track_directions" +  gib_param.track_directions)
        area = area + ppa.ppa_gib.getgibarea(numTrack, gib_param.num_iopin_list, pin2pinList, pin2trackList, track2pinList, gib_param.track_directions, track_reged, maxII)
//        println("numTrack" + numTrack)
//        println("gib_param.num_iopin_list" + gib_param.num_iopin_list)
//        println("gib_param.fcList" + gib_param.fcList)
//        println("gib_param.pin2pinSelList" +  gib_param.pin2pinSel)
//        println("gib_param.pin2trackSel" + gib_param.pin2trackSel)
//        println("gib_param.track_reged" +  gib_param.track_reged)
//        println("gibsParam(i)(j).track_reged" + gibsParam(i)(j).track_reged)
//        println("gib_param.track_directions" +  gib_param.track_directions)
//        println()
//        println("area is :" + area)
      }
    }
    total_area += area
    println("GIB total area :" + area)

    area = 0
    for (i <- 0 until rows) {
      for (j <- 0 until 2) {
        val lsu_type = lsu_posmap(i, j)
        val lsu_pram = lsu_typemap(lsu_type)
        area = area + ppa.ppa_lsu.getlsuarea(lsu_pram.num_Perin, lsu_pram.max_delay, maxII)

      }
    }

    total_area += area
    println("LSU total area :" + area)
    println("CGRA total area :" + total_area)
    total_area

    val writer = new PrintWriter(new File("area.txt"))
    writer.write(total_area.toString)
    writer.close()
  }
}