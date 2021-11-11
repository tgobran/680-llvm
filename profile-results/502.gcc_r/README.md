## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. sbitmap_a_or_b (bitmap.c)
Basic Block Executions: 499227316

Features:
 - one level loop 
 - two level nested conditionals inside loop


### 2. bitmap.c:bitmap_find_bit (bitmap.c)
Basic Block Executions: 487201171

Features:
 - several if then elses
 - one level loop inside if statements

### 3. cfgrtl.c:rtl_split_edge (cfgrtl.c)
Basic Block Executions: 269079181

Features:
  - several if statements
  - one level loop inside if
  - function calls inside if

### 4. bitmap_ior_into (bitmap.c)
Basic Block Executions: 212348411

Features:
  - one level loop
  - if statements inside loop
  - function calls inside loop

### 5. bitmap_bit_p (bitmap.c)
Basic Block Executions: 178942441

Features:
  - only one if statement
  - only one function call

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
