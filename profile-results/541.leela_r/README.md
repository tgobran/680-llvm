## Refrate O2 Hot Functions

For Runtime information the following options were used:
```
-g -O2 -pg -march=native -fno-unsafe-math-optimizations -fcommon
```

Additional instrumentation information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate
```

Active pass information was gathered with the following options:
```
-g -O2 -mllvm --print-changed=quiet -mllvm --filter-print-funcs=FUNCNAME -march=native -fno-unsafe-math-optimizations -fcommon
```

### 1. update_board_fast
Refrate Runtime: 8.22% \
Calls: 791662684

Features:
- TODO

Active Passes:
- TODO

### 2. play_random_move (FastState.cpp)
Refrate Runtime: 8.19% \
Calls: 818055991

Features:
- Nested if statements with function call inside.
- One level for loop within the if condition.

Active Passes:
- TODO

### 3. self_atari
Refrate Runtime: 7.65% \
Calls: 4213754285

Features:
- TODO

Active Passes:
- TODO
