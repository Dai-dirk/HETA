HETA-Mapper
=======================

Temporal CGRA mapping flow including scheduling, binding, configuration generation, and visualization.


## Getting Started

### Dependencies

#### CMake

#### C++-11


### Build

Using the script build.sh
```sh
./build.sh
```

### Run

Using the script run.sh
```sh
./run.sh
```
#### Input files for the mapper

    -p (do not change): the operations set supported by the UPTRA, if a DFG contains the operation not included in this file, it can not be mapped. However, the actual operation that the current hardware supports should refer to the ADG file.

    -a: ADG file of the HETA CGRA, which can be changed as you need.

    -d: Benchmark file, which can be changed as you need.





