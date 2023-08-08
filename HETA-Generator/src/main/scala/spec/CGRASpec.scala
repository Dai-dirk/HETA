 package spec
 // Architecture Specification

 import scala.collection.mutable
 import scala.collection.mutable.ListBuffer
 import common.MacroVar.{NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST}
 import ir._

 // GPE Spec to support heterogeneous GPEs
 class GpeSpec(num_rf_reg_ : Int = 1, max_delay_ : Int = 4,
               operations_ : ListBuffer[String] = ListBuffer("PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR"),
               fromdir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
               todir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)) {
   var num_rf_reg = num_rf_reg_ // number of registers in Regfile
   var max_delay = max_delay_ // max delay cycles of the DelayPipe
   var operations = operations_ // supported operations
   var from_dir = fromdir_ // which directions the GPE inputs are from
   var to_dir = todir_ // which directions the GPE outputs are to
 }

 object GpeSpec {
   def apply(num_rf_reg_ : Int = 1, max_delay_ : Int = 4,
             operations_ : ListBuffer[String] = ListBuffer("ADD"),
             fromdir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
             todir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)) = {
     new GpeSpec(num_rf_reg_, max_delay_, operations_, fromdir_, todir_)
   }
 }

 // LSU Spec to support heterogeneous GPEs
 class LsuSpec(max_delay_ : Int = 4,
               num_Perin_ : Int = 1,
               numOutmem_ : Int = 1,
               mem_depth_ : Int = 8) {
   var max_delay = max_delay_ // max delay cycles of the DelayPipe
   var num_Perin = num_Perin_ // supported operations
   var numOutmem = numOutmem_ // which directions the GPE inputs are from
   var mem_depth = mem_depth_ // which directions the GPE outputs are to
 }

 object LsuSpec {
   def apply(max_delay_ : Int = 4,
             num_Perin_ : Int = 1,
             numOutmem_ : Int = 1,
             mem_depth_ : Int = 8) = {
     new LsuSpec(max_delay_, num_Perin_, numOutmem_, mem_depth_)
   }
 }

 // GIB Spec to support heterogeneous GIBs
 class GibSpec(pin2pin_ : Map[String, Int] , pin2track_ :Map[String, Int]
               , track2pin_ :Map[String, Int]) {
    var pin2pin = pin2pin_
    var pin2track = pin2track_
    var track2pin = track2pin_
 }


 object GibSpec {
   def apply(pin2pin_ : Map[String, Int], pin2track_ : Map[String, Int]
             , track2pin_ : Map[String, Int])= {
     new GibSpec(pin2pin_, pin2track_, track2pin_)
   }
 }

 // CGRA Specification
 object CGRASpec{
   val opin_2_ipin_connect_flexibility = Map(
     "num_ipin_side1" -> 2,
     "num_ipin_side2" -> 2,
     "num_ipin_side3"   -> 2
   )
   val opin_2_otrack_connect_flexibility = Map(
     "num_otrack_side1" -> 1,
     "num_otrack_side2" -> 1,
     "num_otrack_side3" -> 1,
     "num_otrack_side4" -> 1
   )
   val itrack_2_ipin_connect_flexibility = Map(
     "num_itrack_side1" -> 1,
     "num_itrack_side2" -> 1,
     "num_itrack_side3" -> 1,
     "num_itrack_side4" -> 1
   )
   val attrs: mutable.Map[String, Any] = mutable.Map(
     "num_row" -> 4,
     "num_colum" -> 4,
     "data_width" -> 32,
     "cfg_data_width" -> 64,
     "cfg_addr_width" -> 8,
     "cfg_blk_offset" -> 2,
     "num_rf_reg" -> 1,
     "memory_depth" -> 256,
     "num_mem_perin" -> 2,
     "num_mem_out" -> 1,
     "max_II" -> 8,
     "operations" -> ListBuffer("PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR", "SEL"),
     "max_delay" -> 4,
     "gpe_in_from_dir" -> List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
     "gpe_out_to_dir" -> List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
     "num_track" -> 3,
     "track_reged_mode" -> 1,
     "opin_2_ipin_connect_flexibility" -> opin_2_ipin_connect_flexibility,
     "num_output_ib" -> 1,
     "num_input_ob" -> 2,
     "num_input_ib" -> 1,
     "num_output_ob" -> 1,
     "opin_2_otrack_connect_flexibility" -> opin_2_otrack_connect_flexibility,
     "itrack_2_ipin_connect_flexibility" -> itrack_2_ipin_connect_flexibility
   )

   // set default values from attr
   // the attributes in attrs are used as default values

   //GPE
   def setDefaultGpesSpec(): Unit = {
     val gpes_spec = ListBuffer[ListBuffer[GpeSpec]]()
     for (i <- 0 until attrs("num_row").asInstanceOf[Int]) {
       gpes_spec.append(new ListBuffer[GpeSpec])
       for (j <- 0 until attrs("num_colum").asInstanceOf[Int]) {
         val num_rf_reg = attrs("num_rf_reg").asInstanceOf[Int]
         val max_delay = attrs("max_delay").asInstanceOf[Int]
         val operations = attrs("operations").asInstanceOf[ListBuffer[String]]
         val fromdir = attrs("gpe_in_from_dir").asInstanceOf[List[Int]]
         val todir = attrs("gpe_out_to_dir").asInstanceOf[List[Int]]
         gpes_spec(i).append(GpeSpec(num_rf_reg, max_delay, operations, fromdir, todir))
       }
     }
     attrs("gpes") = gpes_spec
   }

   //LSU
   def setDefaultLsusSpec(): Unit = {
     val lsus_spec = ListBuffer[ListBuffer[LsuSpec]]()
     for (i <- 0 until attrs("num_row").asInstanceOf[Int]) {
       lsus_spec.append(new ListBuffer[LsuSpec])
       for (j <- 0 until 2) {
         val max_delay = attrs("max_delay").asInstanceOf[Int]
         val num_Perin = attrs("num_mem_perin").asInstanceOf[Int]
         val numOutmem = attrs("num_mem_out").asInstanceOf[Int]
         val mem_depth = attrs("memory_depth").asInstanceOf[Int]
         lsus_spec(i).append(LsuSpec(max_delay, num_Perin, numOutmem, mem_depth))
       }
     }
     attrs("lsus") = lsus_spec
   }

   //GIB
   def setDefaultGibsSpec(): Unit = {
     val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
     for (i <- 0 to attrs("num_row").asInstanceOf[Int]) {
       gibs_spec.append(new ListBuffer[GibSpec])
       for (j <- 0 to attrs("num_colum").asInstanceOf[Int]) {
         val pin2pin = attrs("opin_2_ipin_connect_flexibility").asInstanceOf[Map[String, Int]]
         val pin2track = attrs("opin_2_otrack_connect_flexibility").asInstanceOf[Map[String, Int]]
         val track2pin = attrs("itrack_2_ipin_connect_flexibility").asInstanceOf[Map[String, Int]]
         gibs_spec(i).append(GibSpec(pin2pin, pin2track, track2pin))
       }
     }
     attrs("gibs") = gibs_spec
   }

   setDefaultGpesSpec()
   setDefaultLsusSpec()
   setDefaultGibsSpec()

   def loadSpec(jsonFile : String): Unit ={
     val jsonMap = IRHandler.loadIR(jsonFile)
     var gpes_spec_update = false
     var lsus_spec_update = false
     var gibs_spec_update = false
     for(kv <- jsonMap){
       if(attrs.contains(kv._1)){
         if(kv._1 == "operations") {
           attrs(kv._1) = kv._2.asInstanceOf[List[String]].to[ListBuffer]
         }else if(kv._1 == "opin_2_ipin_connect_flexibility"){
           attrs(kv._1) = mutable.Map() ++ kv._2.asInstanceOf[Map[String, Int]]
         }else if(kv._1 == "opin_2_otrack_connect_flexibility"){
           attrs(kv._1) = mutable.Map() ++ kv._2.asInstanceOf[Map[String, Int]]
         }else if (kv._1 == "itrack_2_ipin_connect_flexibility") {
           attrs(kv._1) = mutable.Map() ++ kv._2.asInstanceOf[Map[String, Int]]
         }else if (kv._1 == "gpes") {
           gpes_spec_update = true
           val gpe_2d = kv._2.asInstanceOf[List[List[Any]]]
           val gpes_spec = ListBuffer[ListBuffer[GpeSpec]]()
           for (i <- gpe_2d.indices) {
             gpes_spec.append(new ListBuffer[GpeSpec])
             val gpe_1d = gpe_2d(i)
             for (j <- gpe_1d.indices) {
               val gpemap = gpe_1d(j).asInstanceOf[Map[String, Any]]
               val num_rf_reg = gpemap("num_rf_reg").asInstanceOf[Int]
               val max_delay = gpemap("max_delay").asInstanceOf[Int]
               val operations = gpemap("operations").asInstanceOf[List[String]].to[ListBuffer]
               val fromdir = gpemap("from_dir").asInstanceOf[List[Int]]
               val todir = gpemap("to_dir").asInstanceOf[List[Int]]
               gpes_spec(i).append(GpeSpec(num_rf_reg, max_delay, operations, fromdir, todir))
             }
           }
           attrs("gpes") = gpes_spec
         }else if(kv._1 == "lsus"){
           lsus_spec_update = true
           val lsu_2d = kv._2.asInstanceOf[List[List[Any]]]
           val lsus_spec = ListBuffer[ListBuffer[LsuSpec]]()
           for(i <- lsu_2d.indices) {
             lsus_spec.append(new ListBuffer[LsuSpec])
             val lsu_1d = lsu_2d(i)
             for(j <- lsu_1d.indices){
               val lsumap = lsu_1d(j).asInstanceOf[Map[String, Any]]
               val max_delay = lsumap("max_delay").asInstanceOf[Int]
               val num_Perin = lsumap("num_mem_perin").asInstanceOf[Int]
               val numOutmem = lsumap("num_mem_out").asInstanceOf[Int]
               val mem_depth = lsumap("memory_depth").asInstanceOf[Int]
               lsus_spec(i).append(LsuSpec(max_delay, num_Perin, numOutmem, mem_depth))
             }
           }
           attrs("lsus") = lsus_spec
         }else if(kv._1 == "gibs"){
           gibs_spec_update = true
           val gib_2d = kv._2.asInstanceOf[List[List[Any]]]
           val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
           for (i <- gib_2d.indices) {
             gibs_spec.append(new ListBuffer[GibSpec])
             val gib_1d = gib_2d(i)
             for (j <- gib_1d.indices) {
               val gibmap = gib_1d(j).asInstanceOf[Map[String, Any]]
               val pin2pin = gibmap("opin_2_ipin_connect_flexibility").asInstanceOf[Map[String, Int]]
               val pin2track = gibmap("opin_2_otrack_connect_flexibility").asInstanceOf[Map[String, Int]]
               val track2pin = gibmap("itrack_2_ipin_connect_flexibility").asInstanceOf[Map[String, Int]]
                 gibs_spec(i).append(GibSpec(pin2pin, pin2track, track2pin))
             }
           }
           attrs("gibs") = gibs_spec
         }else{
           attrs(kv._1) = kv._2
         }
       }
     }
     if (gpes_spec_update == false) { // set default values
       setDefaultGpesSpec()
     }
     if (lsus_spec_update == false) { // set default values
       setDefaultLsusSpec()
     }
     if (gibs_spec_update == false) { // set default values
       setDefaultGibsSpec()
     }
   }

   def dumpSpec(jsonFile : String): Unit={
     IRHandler.dumpIR(attrs, jsonFile)
   }

 }


