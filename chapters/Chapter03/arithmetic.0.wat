(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (func (;0;) (type 0) (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add)
  (func (;1;) (type 0) (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.sub)
  (func (;2;) (type 0) (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.mul)
  (export "add" (func 0))
  (export "sub" (func 1))
  (export "mul" (func 2)))
