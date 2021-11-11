## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. lz_encoder_mf.c:bt_find_func (liblzma/lz/lz_encoder_mf.c)
Basic Block Executions: 6329676010

Features:
  - if inside loop
  - two level nested loops with several nested ifs inside the inner loop


### 2. lz_encoder_mf.c:bt_skip_func (liblzma/lz/lz_encoder_mf.c)
Basic Block Executions: 5133904873

Features:
 - compeletly like prev one

### 3. lzma_encoder_optimum_normal.c:rc_bit_price (liblzma/rangecoder/price.h)
Basic Block Executions: 2354835913

Features:
  - only calls another hot function

### 4. lzma_encoder_optimum_normal.c:helper2 (liblzma/lzma/lzma_encoder_optimum_normal.c)
Basic Block Executions: 1880543368

Features:
   - nested if statements
   - function calls inside if
   - loops inside if 
   - if with complicated conditions
   - two level nested loops with function call inside the inner one
   - several loops (pnot nested but in sequence) inside a loop

### 5. lzma_encoder_optimum_normal.c:get_literal_price (liblzma/lzma/lzma_encoder_optimum_normal.c)
Basic Block Executions: 1717654358

Features:
  - calling another hot function inside a loop
  - function call inside if
  - loop inside if

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
