## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

This benchmark in particular was difficult to profile for larger functions as the vast majority of the profile data related to very small operator overloads and constructors / get operations.

### 1. shiftup (simulator/cmessageheap.cc)
Basic Block Executions: 5757400812

Features:
* Relatively simple function
* Single nested loop.
* Iteration count derived from a function argument and a global variable.
* Minor control flow inside of the loop.

### 2. insert (simulator/cmessageheap.cc)
Basic Block Executions: 1730030891

Features:
* Two Single Nested Loops
* Loop bodies are simple, the seconde has control flow inside of the loop.

### 3. printState (model/EtherMAC.cc)
Basic Block Executions: 894348966

Features:
* Two Switch statements that perform basic calculation .

### 4. deliver (simulator/cgate.cc)
Basic Block Executions: 599148757

Features:
* Control flow function
* Calls other functions within.

### 5. printAddressTable (model/MACRelayUnitBase.cc)
Basic Block Executions: 556984284

Features:
* Single Nested Loop
* Simple computation inside of the loop with no minor control flow.

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
