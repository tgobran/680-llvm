## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. variance (image_validator/ImageValidator.c)
Basic Block Executions: 1843200

Features:
* Simple function
* Single nested loop
* Iteration count dependent on function arguments
* Loop computation based on strided array access.

### 2. mean (image_validator/ImageValidator.c)
Basic Block Executions: 1843200

Features:
* Simple function
* Single nested loop
* Iteration count dependent on function arguments
* Loop computation based on strided array access.

### 3. read_yuv_file (image_validator/ImageValidator.c)
Basic Block Executions: 921600

Features:
* TODO

### 4. computeSSIM (image_validator/ImageValidator.c)
Basic Block Executions: 921600

Features:
* TODO

### 5. covariance (image_validator/ImageValidator.c)
Basic Block Executions: 921600

Features:
* Simple function
* Single nested loop
* Iteration count dependent on function arguments
* Loop computation based on strided array access.

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
