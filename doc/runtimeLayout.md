### mmap page for code section

### malloc page for operand stack

- X28 point the next empty address of runtime operand stack

### malloc page for function index to signature index mapping

- array for funcType mismatch checking of call_indirect at runtime.  
  -> pop i32_i in operand stack  
  -> get tab.elem[𝑖] as funcIndex  
  -> array[funcIndex] -> signatureIndex  
  -> check with expected signatureIndex(mapped in compile-time)  
  -> if(mismatch) -> Trap

### malloc page for function index to function address mapping

- Also for call_indirect.  
  array[funcIndex] -> its Address at runtime
