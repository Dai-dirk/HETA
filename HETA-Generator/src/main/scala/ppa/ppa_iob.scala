package ppa

import chisel3.util.log2Ceil

object ppa_iob {
  def getiobarea(innum : Int , outnum: Int, maxII: Int): Double ={
    val muxarea = outnum*sm_area.area_mux32_map(innum)
    val cfgarea = outnum*log2Ceil(innum)*sm_area.area_cfg32_map(maxII) * sm_area.cfg_rate
    val area = muxarea + cfgarea
    println("iob area :" + area)
    area
  }

}

