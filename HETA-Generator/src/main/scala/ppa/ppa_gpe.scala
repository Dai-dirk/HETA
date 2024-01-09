package ppa

import chisel3.util.log2Ceil
import op.OPC
//import ppa.area_par.op_group

import scala.collection.mutable.ListBuffer

object ppa_gpe {

  def getgpearea(operations :ListBuffer[String] , num_input_per_operand:ListBuffer[Int] ,max_delay :Int , maxII : Int) : Double = { //num pairs of IO ports in SharedDelayPipe
    val muxarea = num_input_per_operand.map{muxindex => sm_area.area_mux32_map(muxindex + 2)}.reduce(_ +_)
    val delayarea = num_input_per_operand.size * sm_area.area_delay32_map(max_delay)
    //val delayarea =   area_par.area_delay32_map(num_input_per_operand.size)(max_delay)
//    val aluarea = operations.foldLeft(0){(a,b)=> a+ area_par.area_alu32_map(b)}
//    val op_Dif_Gtoup : ListBuffer[String] = new ListBuffer()
//    operations.map{ op => {
//      var inGroup : Boolean = false
//      op_group.map{
//        case(ks ,v) => {
//          if(ks.contains(op)){
//            inGroup = true
//            if(!op_Dif_Gtoup.contains(v)) op_Dif_Gtoup.append(v)
//          }
//        }
//      }
//      if(!inGroup) op_Dif_Gtoup.append(op)
//    }}
    //println("op_Dif_Gtoup: " + op_Dif_Gtoup)
    //println("operations: " + operations)
    //val aluarea = op_Dif_Gtoup.foldLeft(0){(a,b)=> a+ area_par.area_alu32_map(b)}
    val aluarea = operations.foldLeft(0){(a,b)=> a+ sm_area.area_alu32_map(b)} * sm_area.alu_rate
    val constarea = sm_area.area_const32
    val rfarea = sm_area.area_rf32


    val constCfgWidth = 32 // constant
    val aluCfgWidth = log2Ceil(OPC.numOPC) // ALU Config width
    val rfCfgWidth = 0  // RF
    val delayCfgWidthEach = log2Ceil(max_delay+1) // DelayPipe Config width
    val delayCfgWidth = num_input_per_operand.size * delayCfgWidthEach
    val imuxCfgWidthList = num_input_per_operand.map{ mux => log2Ceil(mux + 2) } // input Muxes
    val imuxCfgWidth = imuxCfgWidthList.sum
    val sumCfgWidth = constCfgWidth + aluCfgWidth + rfCfgWidth + delayCfgWidth + imuxCfgWidth
    val cfgarea = sumCfgWidth* sm_area.area_cfg32_map(maxII)* sm_area.cfg_rate
    //println("sumCfgWidth: " + sumCfgWidth)
    val area = muxarea + delayarea +  aluarea + constarea + rfarea + cfgarea
    //println("gpe area :" + area + " muxarea: " + muxarea +  " delayarea: " + delayarea + " aluarea: " + aluarea + " rfarea: " +
     // rfarea + " cfgarea: " + cfgarea)
    area

  }

}
