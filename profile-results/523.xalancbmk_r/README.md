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

Function compile time information was gathered with the following options:
```
-g -O2 -march=native -fno-unsafe-math-optimizations -fcommon -ftime-trace
```

### 1. \<XStringCached\>destroyObject (xalanc/PlatformSupport/ReusableArenaAllocator.hpp)
Refrate Runtime:  45.85% \
Refrate Compile Time: TODO \
Calls: 67113825

Features:
- No Loops
- Multiple calls to constructors and other functions inside.

Active Passes:
- TODO

### 2. contains (ValueStore.cpp)
Refrate Runtime:  19.47% \
Refrate Compile Time: TODO \
Calls: 317186

Features:
- TODO

Active Passes:
- TODO

### 3. release (XalanDOMStringCache.cpp)
Refrate Runtime:  7.34% \
Refrate Compile Time: TODO \
Calls: 67596232

Features:
- TODO

Active Passes:
- TODO
