## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

### 1. play_random_move (FastState.cpp)
Basic Block Executions: 7835753190

Features:
1.	Nested if statements with function call inside.
2.	One level for loop within the if condition.


### 2. kill_or_connect (FastBoard.cpp)
Basic Block Executions: 7547852578

Features:
One level for loop with if condition.


### 3. nbr_criticality (FastBoard.cpp)
Basic Block Executions: 6538040572

Features:
If condition within one level for loop.

### 4. add_pattern_moves (FastBoard.cpp)
Basic Block Executions: 6282723048

Features:
One level for loop with nested if condition.

### 5. merge_strings (FastBoard.cpp)
Basic Block Executions: 5601060928

Features:
Nested for loop within do while loop.

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
