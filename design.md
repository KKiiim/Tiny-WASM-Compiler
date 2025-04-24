### R28 For JIT OperandStack Addr
Use r28 storage as JIT runtime operand stack top address. No validation, assume OperandStack always safe.  
Compiler will check the matching of local.get/local.set stuff before emit jit code.

