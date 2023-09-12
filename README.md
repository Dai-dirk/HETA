# HETA: A Heterogeneous Temporal CGRA Modeling and Design Space Exploration via Bayesian Optimization
[![standard-readme compliant](https://camo.githubusercontent.com/f116695412df39ab3c98d8291befdb93af123f56aecc79fff4b20c410a5b54c7/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f726561646d652532307374796c652d7374616e646172642d627269676874677265656e2e7376673f7374796c653d666c61742d737175617265)](https://github.com/Dai-dirk/HETA/blob/main/README.md)

## Key Futures

HETA is an open-source temporal CGRA modeling framework, including Chisel-based temporal CGRA modeling, C++-based Mapper, and BO-based DSE process.

File catalog:

    HETA-Generator: CGRA modeling and generation based on Chisel. Design a flexible temporal CGRA template, and generate architecture IR (ADG) and Verilog.

    HETA-Mapper: Temporal CGRA mapper, which incorporates the proposed PiPMap.

    HETA-DSE: The BO-based DSE process of HETA.

    benchmark: The 8 test cases used in the paper.

    HETA-ARCH: The generated files related to the temporal CGRA modeling.
      ADG: The ADG files used by the HETA mapper, including HETA-HOMO, HETA-PE, and HETA.
      Verilog: The generated Verilog files, including HETA-HOMO, HETA-PE, and HETA.

    N2N-ARCH: The Verilog file of N2N-based CGRA. 

    Switch-based-ARCH: The Verilog file of Switch-based CGRA. 



### Built With

| environment | version   |
| ----------- | --------- |
| Python      | ≥ 3.7 |
| C++        |   11   |
| JDK       |   ≥ 8   |



## Table of Contents

- [Getting Started]

  - [Install](#Install)

  - [Usage](#Usage)


## Getting Started

### Install

#### Clone the repository

    git clone https://github.com/Dai-dirk/HETA.git

#### Install Optuna

    Please refer to https://github.com/optuna/optuna for Optuna installation.


### Usage

#### HETA-Generator

##### Build and run

    1. Using the script run.sh
      cd HETA-Generator
      ./run.sh

    2. Or using sbt command
      cd HETA-Generator
      sbt "runMain mg.CGRAMG -td ./test_run_dir"

#### HETA-Mapper

##### Build

    Using the script build.sh
      cd HETA-Mapper
      ./build.sh

##### Run

    Using the script run.sh
      ./run.sh

#### HETA-DSE

##### Homogeneous DSE

    python dse-tpe.py

##### Heterogeneous DSE

    python heteDSE.py

### Example

#### An example of HETA-Mapper

![ ](https://github.com/Dai-dirk/HETA/blob/main/example.png)


      

