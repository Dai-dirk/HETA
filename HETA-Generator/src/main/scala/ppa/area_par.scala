//package ppa
//
//object area_par {
//  //  val area_mux32_map :Map[Int,Int] = Map(
//  //    1 ->0,
//  //    2 ->115,
//  //    3->143,
//  //    4->215,
//  //    5->277,
//  //    6->316,
//  //    7->386,
//  //    8->421,
//  //    9->509,
//  //    10->572,
//  //    11->607,
//  //    12->690,
//  //    13->740,
//  //    14->776,
//  //    15->834
//  //
//  //  )
//  val area_mux32_map :Map[Int,Int] = Map(
//    1 ->30,
//    2 ->78,
//    3->103,
//    4->152,
//    5->203,
//    6->231,
//    7->273
//
//  )
//
//  //  val area_delay32_map : Map[Int, Int] = Map(
//  //    1 ->867,
//  //    2-> 1322,
//  //    3->1700,
//  //    4 -> 2157,
//  //    5 ->  2555,
//  //    6 ->2926,
//  //    7 ->  3315
//  //
//  //  )
////Map[num,[maxDelay,area]] num: number of operations
//  val area_delay32_map : Map[Int, Map[Int,Int]] = Map(
//    1 -> Map(
//      1 -> 575,
//      2-> 906,
//      3-> 1178,
//      4 -> 1598,
//      5 ->  1771,
//      6 -> 2026,
//      7 ->  2273
//    ),
//    2 -> Map(
//      1 -> 1891,
//      2 -> 2388,
//      3 -> 3056,
//      4 -> 3665,
//      5 -> 4105,
//      6 -> 4649,
//      7 -> 5171
//    ),
////    1 ->878,
////    2-> 1332,
////    3->1736,
////    4 -> 2192,
////    5 ->  2592,
////    6 ->2979,
////    7 ->  3396
//
//  )
//
//  //  val area_alu32_map : Map[String, Int] = Map(
//  //    "PASS" -> 60,
//  //    "ADD" -> 402,
//  //    "SUB" -> 340,
//  //    "MUL" -> 3895,
//  //    "AND" -> 83,
//  //    "OR" ->83 ,
//  //    "XOR" ->118 ,
//  //    "SHL" -> 445,
//  //    "LSHR" -> 453,
//  //    "ASHR" -> 459 ,
//  //    "EQ" -> 137,
//  //    "NE" -> 137,
//  //    "LT" -> 189,
//  //    "LE" -> 187,
//  //    "SEL" -> 109
//  //  )
//
//  //  val area_alu32_map : Map[String, Int] = Map(
//  //      "PASS" -> 60,
//  //      "ADD" -> 402,
//  //      "SUB" -> 340,
//  //      "MUL" -> 3895,
//  //      "AND" -> 83,
//  //      "OR" ->83 ,
//  //      "XOR" ->118 ,
//  //      "SHL" -> 445,
//  //      "LSHR" -> 453,
//  //      "ASHR" -> 459 ,
//  //      "EQ" -> 137,
//  //      "NE" -> 137,
//  //      "LT" -> 189,
//  //      "LE" -> 187,
//  //      "SEL" -> 109
//  //    )
// val op_group : Map [List[String] ,String] = Map(
//    ( List("ADD","SUB" )->"group1"),
//    ( List("AND","OR","XOR" )->"group2"),
//    ( List("SHL","LSHR","ASHR" )->"group3"),
//    ( List("MUL" )->"group4"),
//    ( List("EQ","NE","LT","LE" )->"group5"),
//    ( List("PASS" )->"group6")
//  );
//  val area_alu32_map : Map[String, Int] = Map(
//    "PASS" -> 51,
//    "ADD" -> 234,
//    "SUB" -> 243,
//    "MUL" -> 2498,
//    "AND" -> 59,
//    "OR" ->59 ,
//    "XOR" ->88 ,
//    "SHL" -> 313,
//    "LSHR" -> 313,
//    "ASHR" -> 327 ,
//    "EQ" -> 117,//
//    "NE" -> 117,
//    "LT" -> 145,//
//    "LE" -> 146,
//    "SEL" -> 70,
//    "group1" -> 243,
//    "group2" -> 88,
//    "group3" -> 327,
//    "group4" -> 2498,
//    "group5" -> 146,
//    "group6" -> 51
//  )
//  val area_regnxt32 = 151 //暂时先忽略
//  //  val area_rf32 = 338 //目前所有rf都是一个把
//  val area_rf32 = 251
//
//  val area_cfgpre32 = 80 * 1.07
//
//  val area_const32 = 0
//
//  val cycle = 5
//  val reduce_rate = 0.0
//
//  val alu_rate = 1.1
//
//  // for lsu
//  val area_sram_32x64 = 17204 // the depth is 64
//
//}
//
