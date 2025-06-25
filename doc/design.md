### R28 For JIT OperandStack Addr

Use r28 storage as JIT runtime operand stack top address. No validation, assume OperandStack always safe.  
Compiler will check the matching of local.get/local.set stuff before emit jit code.

### Max Support 8 Params WASM Function

##### In Demo

Params with hardcode cast type for `singleCallByName`.

##### In Spec test

    Use static 8 64bits regiters for C++ call wasm. In wasm, fetch the needed size from register.
    Because it is not possible to do dynamic type distribution at compile time in C++.

### OPCode::IF ELSE

```
Two cases:
IF->B.c->TRUE_BLOCK->B->ELSE->FALSE_BLOCK->END->OTHER
IF->B.c->TRUE_BLOCK->OTHER
```

## Some workaround

- Assumed RETURN will not appear in if/else branch
- Explicit OPCode::Return in wat will cause another ret emitted at FUNC_END

## TODO

1. Only save dirty register before call. Need record register usage reference
2. Remove JIT runtime operandStack can reduce memory access a lot. Maintain some reference in Compiler level.
3. Use memory+ptr to increase function params
4. Use RAII for ins_sp/dec_sp, may do it in stackFrame class
