package dsa

import scala.io.Source
import java.io._
import java.math.BigInteger
import scala.math.BigInt
import chisel3._
import chisel3.util._

import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import chisel3.iotesters.{ChiselFlatSpec, Driver, PeekPokeTester}
import spec.CGRASpec

class CGRATest(c: CGRA, cfgFilename: String) extends PeekPokeTester(c) {
  val rows = c.param.rows
  val cols = c.param.rows
  // read config bit file
  Source.fromFile(cfgFilename).getLines().foreach{
    line => {
      val items = line.split(" ")
      val addr = Integer.parseInt(items(0), 16);        // config bus address
      val data = BigInt(new BigInteger(items(1), 16));  // config bus data
      poke(c.io.cfg_en, 1)
      poke(c.io.cfg_addr, addr)
      //println("addr"+addr)
      poke(c.io.cfg_data, data)
      step(1)
    }
  }
  // delay for config done
  step(c.cfgRegNum + 2)
  poke(c.io.cfg_en, 0)

  poke(c.io.hostInterface(0).read_data.ready, 0)
  poke(c.io.hostInterface(1).read_data.ready, 0)
  poke(c.io.hostInterface(2).read_data.ready, 0)
  poke(c.io.hostInterface(3).read_data.ready, 0)
  poke(c.io.hostInterface(4).read_data.ready, 0)
  poke(c.io.hostInterface(5).read_data.ready, 0)
  poke(c.io.hostInterface(6).read_data.ready, 0)
  poke(c.io.hostInterface(7).read_data.ready, 0)
  //send CPU data to LSU
  poke(c.io.hostInterface(5).write_data.valid, 1)//N0
  poke(c.io.hostInterface(5).write_data.bits, 1)
  poke(c.io.hostInterface(5).write_addr, 1)
  poke(c.io.hostInterface(5).cycle, 0)
  step(1)
  poke(c.io.hostInterface(3).write_data.valid, 1)//N1
  poke(c.io.hostInterface(3).write_data.bits, 1)
  poke(c.io.hostInterface(3).write_addr, 1)
  poke(c.io.hostInterface(3).cycle, 0)
  step(1)
  poke(c.io.hostInterface(1).write_data.valid, 1)//N2
  poke(c.io.hostInterface(1).write_data.bits, 1)
  poke(c.io.hostInterface(1).write_addr, 1)
  poke(c.io.hostInterface(1).cycle, 2)
  step(1)
  poke(c.io.hostInterface(7).write_data.valid, 1)//N3
  poke(c.io.hostInterface(7).write_data.bits, 1)
  poke(c.io.hostInterface(7).write_addr, 1)
  poke(c.io.hostInterface(7).cycle, 0)
  step(1)
  poke(c.io.hostInterface(7).write_data.valid, 1) //N4
  poke(c.io.hostInterface(7).write_data.bits, 1)
  poke(c.io.hostInterface(7).write_addr, 1)
  poke(c.io.hostInterface(7).cycle, 1)
  step(1)
  poke(c.io.hostInterface(4).write_data.valid, 1) //N5
  poke(c.io.hostInterface(4).write_data.bits, 2)
  poke(c.io.hostInterface(4).write_addr, 1)
  poke(c.io.hostInterface(4).cycle, 1)
  step(1)
  poke(c.io.hostInterface(3).write_data.valid, 1) //N6
  poke(c.io.hostInterface(3).write_data.bits, 1)
  poke(c.io.hostInterface(3).write_addr, 1)
  poke(c.io.hostInterface(3).cycle, 1)
  step(1)
  poke(c.io.hostInterface(6).write_data.valid, 1) //N7
  poke(c.io.hostInterface(6).write_data.bits, 1)
  poke(c.io.hostInterface(6).write_addr, 1)
  poke(c.io.hostInterface(6).cycle, 0)
  step(1)
  poke(c.io.hostInterface(3).write_data.valid, 1) //N8
  poke(c.io.hostInterface(3).write_data.bits, 1)
  poke(c.io.hostInterface(3).write_addr, 1)
  poke(c.io.hostInterface(3).cycle, 2)
  step(1)
  poke(c.io.hostInterface(0).write_data.valid, 1) //N9
  poke(c.io.hostInterface(0).write_data.bits, 1)
  poke(c.io.hostInterface(0).write_addr, 1)
  poke(c.io.hostInterface(0).cycle, 1)
  step(1)
  poke(c.io.hostInterface(0).write_data.valid, 1) //N10
  poke(c.io.hostInterface(0).write_data.bits, 1)
  poke(c.io.hostInterface(0).write_addr, 1)
  poke(c.io.hostInterface(0).cycle, 0)
  step(1)
  poke(c.io.hostInterface(2).write_data.valid, 1) //N11
  poke(c.io.hostInterface(2).write_data.bits, 2)
  poke(c.io.hostInterface(2).write_addr, 1)
  poke(c.io.hostInterface(2).cycle, 0)
  step(1)
  poke(c.io.hostInterface(5).write_data.valid, 1) //N12
  poke(c.io.hostInterface(5).write_data.bits, 1)
  poke(c.io.hostInterface(5).write_addr, 1)
  poke(c.io.hostInterface(5).cycle, 1)
  step(1)
  poke(c.io.hostInterface(1).write_data.valid, 1) //N13
  poke(c.io.hostInterface(1).write_data.bits, 1)
  poke(c.io.hostInterface(1).write_addr, 1)
  poke(c.io.hostInterface(1).cycle, 0)
  step(1)
  poke(c.io.hostInterface(0).write_data.valid, 0)
  poke(c.io.hostInterface(1).write_data.valid, 0)
  poke(c.io.hostInterface(2).write_data.valid, 0)
  poke(c.io.hostInterface(3).write_data.valid, 0)
  poke(c.io.hostInterface(4).write_data.valid, 0)
  poke(c.io.hostInterface(5).write_data.valid, 0)
  poke(c.io.hostInterface(6).write_data.valid, 0)
  poke(c.io.hostInterface(7).write_data.valid, 0)
  step(1)
  /*poke(c.io.hostInterface(1).read_data.ready, 1) //N29
  poke(c.io.hostInterface(1).read_addr, 1)
  poke(c.io.hostInterface(1).cycle, 1)
  step(1)
  val TEST = peek(c.io.hostInterface(1).read_data.bits).toString()
  println("test: " + TEST)
  poke(c.io.hostInterface(1).read_data.ready, 0) //N29*/
  /* poke(c.io.hostInterface(0).read_data.ready, 0)
   poke(c.io.hostInterface(1).read_data.ready, 0)
   poke(c.io.hostInterface(2).read_data.ready, 0)
   poke(c.io.hostInterface(3).read_data.ready, 0)
   poke(c.io.hostInterface(4).read_data.ready, 0)
   poke(c.io.hostInterface(5).read_data.ready, 0)
   poke(c.io.hostInterface(6).read_data.ready, 0)
   poke(c.io.hostInterface(7).read_data.ready, 0)
   step(1)
   //CPU read data from LSU
   poke(c.io.hostInterface(0).read_data.ready, 1) //N29
   poke(c.io.hostInterface(0).read_addr, 1)
   poke(c.io.hostInterface(0).cycle, 2)
   step(1)
   poke(c.io.hostInterface(1).read_data.ready, 1) //N29
   poke(c.io.hostInterface(1).read_addr, 2)
   poke(c.io.hostInterface(1).cycle, 2)
   step(1)
   poke(c.io.hostInterface(2).read_data.ready, 1) //N29
   poke(c.io.hostInterface(2).read_addr, 3)
   poke(c.io.hostInterface(2).cycle, 2)
   step(1)
   poke(c.io.hostInterface(3).read_data.ready, 1) //N29
   poke(c.io.hostInterface(3).read_addr, 1)
   poke(c.io.hostInterface(3).cycle, 2)
   step(1)
   poke(c.io.hostInterface(3).read_data.ready, 1) //N29
   poke(c.io.hostInterface(3).read_addr, 2)
   poke(c.io.hostInterface(3).cycle, 0)
   step(1)
   poke(c.io.hostInterface(1).read_data.ready, 1) //N29
   poke(c.io.hostInterface(1).read_addr, 3)
   poke(c.io.hostInterface(1).cycle, 0)
   step(1)
   poke(c.io.hostInterface(7).read_data.ready, 1) //N29
   poke(c.io.hostInterface(7).read_addr, 1)
   poke(c.io.hostInterface(7).cycle, 0)
   step(1)
   poke(c.io.hostInterface(7).read_data.ready, 1) //N29
   poke(c.io.hostInterface(7).read_addr, 2)
   poke(c.io.hostInterface(7).cycle, 1)
   step(1)
   poke(c.io.hostInterface(5).read_data.ready, 1) //N29
   poke(c.io.hostInterface(5).read_addr, 3)
   poke(c.io.hostInterface(5).cycle, 0)
   step(1)
   val TEST = peek(c.io.hostInterface(0).read_data.bits).toString()
   println("test: " + TEST)*/
  /*poke(c.io.hostInterface(6).read_data.ready, 1)
  poke(c.io.hostInterface(6).read_addr, 11)
  step(1)
  val read=peek(c.io.hostInterface(6).read_data.bits).toString()
  println("read:"+read)*/
  // enable computation
  for( i <- 0 until cols+2){
    poke(c.io.en(i), 1)
  }
  //step(1)
  // input test data
  // refer to mapped_dfgio.txt for the occupied I/O ports
  for( i <- 0 until 15) {
    step(1)
    val ADD14 = peek(c.io.out(2)).toString() //ADD21
    val ADD15 = peek(c.io.out(6)).toString() //ADD22
    val ADD16 = peek(c.io.out(2)).toString() //ADD23
    val ADD17 = peek(c.io.out(6)).toString() //ADD24
    val ADD38 = peek(c.io.out(0)).toString() //ADD21
    val ADD39 = peek(c.io.out(0)).toString() //ADD22
    val ADD40 = peek(c.io.out(2)).toString() //ADD23
    val ADD41 = peek(c.io.out(6)).toString() //ADD24
    println("----------------" + i + "----------------")
    println(" ADD14: " + ADD14 + " ADD15: " + ADD15 + " ADD16: " + ADD16 + " ADD17: " + ADD17)
    println( " ADD38: " + ADD38 + " ADD39: " + ADD39 + " ADD40: " + ADD40 + " ADD41: " + ADD41)
  }
  for (i <- 0 until cols + 2) {
    poke(c.io.en(i), 0)
  }
  step(1)
  //CPU read data from LSU
  poke(c.io.hostInterface(0).read_data.ready, 1) //O0
  poke(c.io.hostInterface(0).read_addr, 6)
  poke(c.io.hostInterface(0).cycle, 2)
  step(1)
  val O0 = peek(c.io.hostInterface(0).read_data.bits).toString()
  poke(c.io.hostInterface(6).read_data.ready, 1) //O1
  poke(c.io.hostInterface(6).read_addr, 7)
  poke(c.io.hostInterface(6).cycle, 1)
  step(1)
  val O1 = peek(c.io.hostInterface(6).read_data.bits).toString()
  poke(c.io.hostInterface(6).read_data.ready, 1) //O2
  poke(c.io.hostInterface(6).read_addr, 6)
  poke(c.io.hostInterface(6).cycle, 2)
  step(1)
  val O2 = peek(c.io.hostInterface(6).read_data.bits).toString()
  poke(c.io.hostInterface(2).read_data.ready, 1) //O3
  poke(c.io.hostInterface(2).read_addr, 7)
  poke(c.io.hostInterface(2).cycle, 1)
  step(1)
  val O3 = peek(c.io.hostInterface(2).read_data.bits).toString()

  println("~~~~~~~~~~~~~~results~~~~~~~~~~~~~~~~")
  println("O0:" + O0 + " O1: " + O1 + " O2: " + O2 + " O3: " + O3 )
  println(" ")
  //}
  //    println(peek(c.io.out(1)).toString())
  //step(5)
  //
  //println("OUTPUT and STORE data:"+read0+","+read1)
  /*for( i <- 1 until 20){
    poke(c.io.in(6), 6)
    poke(c.io.in(7), -50)
//    poke(c.io.in(0), (2*i+1))
    step(1)
    val valid1 = { if(i >= 2) "V: " else "X: " }
    val valid2 = { if(i >= 5) "V: " else "X: " }
    val out1 = peek(c.io.out(6)).toInt
    val out2 = peek(c.io.out(0)).toString()
//    println(peek(c.io.out(1)).toString())
    println(valid1 + out1 + ", " + valid2 + out2)
  }*/
}


object CGRATester extends App {
  val jsonFile = "src/main/resources/cgra_spec.json"
  CGRASpec.loadSpec(jsonFile)
  // val dut = new CGRA(attrs)
  iotesters.Driver.execute(args, () => new CGRA(CGRASpec.attrs, false)) {
    c => new CGRATest(c, "src/main/resources/config.bit")
  }

}


