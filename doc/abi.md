```C++
  template <typename T, typename... Args> T singleCallByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    ...
    ...
    using FuncPtr = T (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(funcStartAddress);
    return func(std::forward<Args>(args)...);
  }
```

## Stack layout

```
=================
LR
Params(store before call)
other locals
=================
```
