## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

Additional instrumentation information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

### 1. regex.c:S_regmatch (ext/re/re_exec.c)
Refrate Runtime: 31.77% \
Calls: 457545339

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

### 2. Perl_regexec_flags (regexec.c)
Refrate Runtime: 7.75% \
Calls: 429181430

Features:
- Several nested if-else statements with complex function calls inside.
- Several nested if conditions with goto statements.
- Several nested if statements with function calls inside.
- A while loop with several nested if statements inside.
- A nested if condition with a while loop inside.

### 3. Perl_hv_common (hv.c)
Refrate Runtime: 6.96% \
Calls: 506576246

Features:
- Several nested if-else statements with complex function calls within.
- Multiple single level loops with if statements inside, some have calls to complex functions.


## Ignored Functions

### Perl_pp_match: 8.78%
- Couldn't be found in the source files for proper function analysis.

### Perl_pp_multideref: 7.96%
- Couldn't be found in the source files for proper function analysis.
