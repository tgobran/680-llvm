## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. sbitmap_a_or_b (bitmap.c)
Basic Block Executions: 499227316

Features:
- several if then else statements
- function declared with two different prototypes by the help of #IFDEF


### 2. bitmap.c:bitmap_find_bit (bitmap.c)
Basic Block Executions: 487201171

Features:
   - function declared with two different prototypes by the help of #IFDEF
   - several one level loops with if statements with complicated condition inside them
   - if statements with function calls as their condition

### 3. cfgrtl.c:rtl_split_edge (cfgrtl.c)
Basic Block Executions: 269079181

Features:
- two level perfect nested loops with a function call inside the second loop.  --> three of this
- function calls inside if statements
- function calls in condition part of the loop
- function calls inside of if statement
- goto !!!!!!!  ----> considerning this goto it's three level nested loop

### 4. bitmap_ior_into (bitmap.c)
Basic Block Executions: 212348411

Features:
 - function declared with two different prototypes by the help of #IFDEF
 - one level loop
 - double nested if statement inside loop

### 5. bitmap_bit_p (bitmap.c)
Basic Block Executions: 178942441

Features:
  - function declared with two different prototypes by the help of #IFDEF
  - malloc --> heap access
  - nested if statements with function calls inside
  - one level loops with if statements inside, some of thos if statments contain function calls inside
  - several if then elses inside loop
  - call another hot function --> qsort
  - if --> loop ----> function call

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
