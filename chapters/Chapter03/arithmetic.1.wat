(module
  (type (;0;) (func (param i64 i64) (result i64)))
  (func (;0;) (type 0) (param i64 i64) (result i64)
    local.get 0
    local.get 1
    i64.add)
  (func (;1;) (type 0) (param i64 i64) (result i64)
    local.get 0
    local.get 1
    i64.sub)
  (func (;2;) (type 0) (param i64 i64) (result i64)
    local.get 0
    local.get 1
    i64.mul)
  (export "add" (func 0))
  (export "sub" (func 1))
  (export "mul" (func 2)))
