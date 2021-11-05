## Refrate O0 Hot Functions

The program was compiled with the following options:

```-g -O0 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

Smaller functions such as simple Constructors were ignored for hot functions.

This benchmark was particularly problematic in terms of finding suitable hot functions.

### 1. stringLen (xercesc/util/XMLString.hpp)
Basic Block Executions: 24090350766

Features:
* TODO

### 2. isInBorders (xalanc/PlatformSupport/ArenaBlockBase.hpp)
Basic Block Executions: 16754842027

Features:
* TODO

### 3. destroyObject (xalanc/PlatformSupport/ReusableArenaBlock.hpp)
Basic Block Executions: 16752607655

Features:
* TODO

### 4. equals (xercesc/util/XMLString.hpp)
Basic Block Executions: 8607499441

Features:
* TODO

### 5. shrinkCount (xalanc/Include/XalanVector.hpp)
Basic Block Executions: 1279868516

Features:
* TODO

## Refrate O2 Hot Functions

The program was compiled with the following options:

```-g -O2 -fno-inline-functions -march=native -fno-unsafe-math-optimizations -fcommon -fprofile-instr-generate```

TODO
