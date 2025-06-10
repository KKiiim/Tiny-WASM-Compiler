(module
  (type (;0;) (func (param i32) (result i32)))
  (func (;0;) (type 0) (param i32) (result i32)
    local.get 0
    if (result i32)  ;; label = @1
      i32.const 7
    else
      i32.const 8
    end)
  (export "singular" (func 0)))
