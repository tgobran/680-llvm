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



### 3. Perl_leave_scope (scope.c)
Basic Block Executions: 995660652

Features:
1.	Nested If statement within while loop.
2.	Several If condition inside switch case
3.	Function defined with #ifdef identifier statement
4.	One level loop with if statements inside
5.	Nested if condition with complex function call inside.
6.	Case statement with multiple if condition.


### 4. Perl_regexec_flags (regexec.c)
Basic Block Executions: 852534058

Features:
1.	Several Nested if then else statement with complex function call inside.
2.	Several Nested If condition with goto statement 
3.	Function call within the nested if statements.
4.	Several Nested if condition within while loop.
5.	While loop inside the nested if condition.


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
