## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

For Basic Block information the following options were used:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

### 1. regex.c:S_regmatch (ext/re/re_exec.c)
Basic Block Executions: 1130348949

Refrate Runtime: 29.09%

Features:
- Single level while loop with complex function call.
- Multiple nested if conditions with function calls inside.
- Several if-else conditions within while loops.
- While loops inside if statements.
- Multiple nexted if-else statements.
- Case statements with multiple if-else statements.
- Several nested if conditions within a single level for loop.
- While condition with function inside a nested loop.
- Nested loops within while loops, sometimes based on conditions.

### 2. Perl_hv_common (hv.c)
Basic Block Executions: 623131659

Refrate Runtime: 8.45%

Features:
- Several nested if-else statements with complex function calls within.
- Multiple single level loops with if statements inside, some have calls to complex functions.

### 3. Perl_regexec_flags (regexec.c)
Basic Block Executions: 852534058

Refrate Runtime: 5.71%

Features:
- Several nested if-else statements with complex function calls inside.
- Several nested if conditions with goto statements.
- Several nested if statements with function calls inside.
- A while loop with several nested if statements inside.
- A nested if condition with a while loop inside.

## Ignored Functions

### Perl_pp_match: 8.97%
- Couldn't be found in the source files for proper function analysis.

### Perl_pp_multideref: 8.81%
- Couldn't be found in the source files for proper function analysis.
