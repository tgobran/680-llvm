## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. remove_one_fast (search.cpp)
Basic Block Executions: 11904211699

Features:
* Conditionally executed Single Nested Loop
* Loop Iteration based on function parameter
* Significant amount of conditional execution in the function

### 2. FindFirstRemove (bits.cpp)
Basic Block Executions: 7592818589

Features:
* Simple function
* Global array access

### 3. PopCount (bits.cpp)
Basic Block Executions: 5393808666

Features:
* Simple function
* Single Nested Loop
* Array iteration dependent on function parameter

### 4. FileAttacks (bitboard.cpp)
Basic Block Executions: 3437787237

Features:
* Simple function
* Global array access

### 5. RankAttacks (bitboard.cpp)
Basic Block Executions: 3330560340

Features:
* Simple function
* Global array access

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
