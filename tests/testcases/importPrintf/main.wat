(module $main.wasm
  (type (;0;) (func (param i32 i32) (result i32)))
  (type (;1;) (func))
  (import "env" "myPrintf" (func $myPrintf (type 0)))
  (func $_start (type 1)
    (local i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    local.set 0
    i32.const 16
    local.set 1
    local.get 0
    local.get 1
    i32.sub
    local.set 2
    local.get 2
    global.set $__stack_pointer
    i32.const 123
    local.set 3
    local.get 2
    local.get 3
    i32.store
    i32.const 1024
    local.set 4
    local.get 4
    local.get 2
    call $myPrintf
    drop
    i32.const 16
    local.set 5
    local.get 2
    local.get 5
    i32.add
    local.set 6
    local.get 6
    global.set $__stack_pointer
    return)
  (func $dummy (type 1))
  (func $__wasm_call_dtors (type 1)
    call $dummy
    call $dummy)
  (func $_start.command_export (type 1)
    call $_start
    call $__wasm_call_dtors)
  (table (;0;) 1 1 funcref)
  (memory (;0;) 1)
  (global $__stack_pointer (mut i32) (i32.const 9248))
  (export "memory" (memory 0))
  (export "__indirect_function_table" (table 0))
  (export "_start" (func $_start.command_export))
  (data $.rodata (i32.const 1024) "[test] -d is %d\0a\00"))
