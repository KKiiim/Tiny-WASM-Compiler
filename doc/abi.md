```C++
  template <typename T, typename... Args> T singleCallByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    ...
    ...
    using FuncPtr = T (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(funcStartAddress);
    return func(std::forward<Args>(args)...);
  }
```

### Register layout when native call WASM function

If args is (iii), x0,x1,x2 will be used as params registers by default. x3 is used as `bl` register.  
Similarly, if args is () which is empty, x0 will be used as `bl` register.  
For conform to the calling convention of C/C++, in JIT code get params from C++ function call with register x0-x7 if used.

## Stack layout

```
=================
FP
Params(store before call)
other locals
=================
```
