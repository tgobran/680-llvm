## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. Perl_runops_standard (run.c)
Basic Block Executions: 4342350552

Features:
* TODO

### 2. regex.c:S_regmatch (ext/re/re_exec.c)
Basic Block Executions: 1130348949

Features:
* TODO

### 3. Perl_leave_scope (scope.c)
Basic Block Executions: 995660652

Features:
* TODO

### 4. Perl_regexec_flags (regexec.c)
Basic Block Executions: 852534058

Features:


### 5. Perl_hv_common (hv.c)
Basic Block Executions: 623131659

Features:

1.	Several Nested If then else statement with complex function calls inside.
2.	Function defined with #ifdef identifier statement
3.	Multiple One level loop with multiple if statements inside. Among those if statements some 
Calls complex functions inside.


## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
