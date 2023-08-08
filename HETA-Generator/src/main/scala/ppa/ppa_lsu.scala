package ppa

import chisel3.util.log2Ceil

object ppa_lsu {

  def getlsuarea(num_perin : Int, maxdelay : Int, maxII : Int) : Double = {
    val muxarea = 2 * sm_area.area_mux32_map(num_perin + 1)
    val delayarea = 2 * sm_area.area_delay32_map(maxdelay)
    val sramarea = sm_area.area_sram_32x64
    val constarea = sm_area.area_const32

    val constCfgWidth = 32 // constant
    val selWidth = log2Ceil(num_perin + 1)
    val delayCfgWidthEach = log2Ceil(maxdelay + 1) // DelayPipe Config width
    val delayCfgWidth = 2 * delayCfgWidthEach
    val sumCfgWidth = 2 + constCfgWidth + delayCfgWidth + selWidth * 2 // opcode-width + delay + sel + const
    val cfgarea = sumCfgWidth * sm_area.area_cfg32_map(maxII)* sm_area.cfg_rate

    val area = muxarea + delayarea + sramarea + constarea + cfgarea
    println("lsu area :" + area)
    area

  }

}
