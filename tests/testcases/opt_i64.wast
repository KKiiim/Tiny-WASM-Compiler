;; i64 operations

(module
  (func (export "add") (param $x i64) (param $y i64) (result i64) (i64.add (local.get $x) (local.get $y)))
  (func (export "sub") (param $x i64) (param $y i64) (result i64) (i64.sub (local.get $x) (local.get $y)))
  (func (export "mul") (param $x i64) (param $y i64) (result i64) (i64.mul (local.get $x) (local.get $y)))
  (func (export "div_s") (param $x i64) (param $y i64) (result i64) (i64.div_s (local.get $x) (local.get $y)))
  (func (export "div_u") (param $x i64) (param $y i64) (result i64) (i64.div_u (local.get $x) (local.get $y)))
  ;; (func (export "rem_s") (param $x i64) (param $y i64) (result i64) (i64.rem_s (local.get $x) (local.get $y)))
  ;; (func (export "rem_u") (param $x i64) (param $y i64) (result i64) (i64.rem_u (local.get $x) (local.get $y)))
  ;; (func (export "and") (param $x i64) (param $y i64) (result i64) (i64.and (local.get $x) (local.get $y)))
  ;; (func (export "or") (param $x i64) (param $y i64) (result i64) (i64.or (local.get $x) (local.get $y)))
  ;; (func (export "xor") (param $x i64) (param $y i64) (result i64) (i64.xor (local.get $x) (local.get $y)))
  ;; (func (export "shl") (param $x i64) (param $y i64) (result i64) (i64.shl (local.get $x) (local.get $y)))
  ;; (func (export "shr_s") (param $x i64) (param $y i64) (result i64) (i64.shr_s (local.get $x) (local.get $y)))
  ;; (func (export "shr_u") (param $x i64) (param $y i64) (result i64) (i64.shr_u (local.get $x) (local.get $y)))
  ;; (func (export "rotl") (param $x i64) (param $y i64) (result i64) (i64.rotl (local.get $x) (local.get $y)))
  ;; (func (export "rotr") (param $x i64) (param $y i64) (result i64) (i64.rotr (local.get $x) (local.get $y)))
  (func (export "clz") (param $x i64) (result i64) (i64.clz (local.get $x)))
  (func (export "ctz") (param $x i64) (result i64) (i64.ctz (local.get $x)))
  ;; (func (export "popcnt") (param $x i64) (result i64) (i64.popcnt (local.get $x)))
  ;; (func (export "extend8_s") (param $x i64) (result i64) (i64.extend8_s (local.get $x)))
  ;; (func (export "extend16_s") (param $x i64) (result i64) (i64.extend16_s (local.get $x)))
  ;; (func (export "extend32_s") (param $x i64) (result i64) (i64.extend32_s (local.get $x)))
  (func (export "eqz") (param $x i64) (result i32) (i64.eqz (local.get $x)))
  (func (export "eq") (param $x i64) (param $y i64) (result i32) (i64.eq (local.get $x) (local.get $y)))
  (func (export "ne") (param $x i64) (param $y i64) (result i32) (i64.ne (local.get $x) (local.get $y)))
  ;; (func (export "lt_s") (param $x i64) (param $y i64) (result i32) (i64.lt_s (local.get $x) (local.get $y)))
  ;; (func (export "lt_u") (param $x i64) (param $y i64) (result i32) (i64.lt_u (local.get $x) (local.get $y)))
  (func (export "le_s") (param $x i64) (param $y i64) (result i32) (i64.le_s (local.get $x) (local.get $y)))
  (func (export "le_u") (param $x i64) (param $y i64) (result i32) (i64.le_u (local.get $x) (local.get $y)))
  ;; (func (export "gt_s") (param $x i64) (param $y i64) (result i32) (i64.gt_s (local.get $x) (local.get $y)))
  (func (export "gt_u") (param $x i64) (param $y i64) (result i32) (i64.gt_u (local.get $x) (local.get $y)))
  ;; (func (export "ge_s") (param $x i64) (param $y i64) (result i32) (i64.ge_s (local.get $x) (local.get $y)))
  (func (export "ge_u") (param $x i64) (param $y i64) (result i32) (i64.ge_u (local.get $x) (local.get $y)))
)

(assert_return (invoke "add" (i64.const 1) (i64.const 1)) (i64.const 2))
(assert_return (invoke "add" (i64.const 1) (i64.const 0)) (i64.const 1))
(assert_return (invoke "add" (i64.const -1) (i64.const -1)) (i64.const -2))
(assert_return (invoke "add" (i64.const -1) (i64.const 1)) (i64.const 0))
(assert_return (invoke "add" (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x8000000000000000))
(assert_return (invoke "add" (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x7fffffffffffffff))
(assert_return (invoke "add" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i64.const 0))
(assert_return (invoke "add" (i64.const 0x3fffffff) (i64.const 1)) (i64.const 0x40000000))

(assert_return (invoke "sub" (i64.const 1) (i64.const 1)) (i64.const 0))
(assert_return (invoke "sub" (i64.const 1) (i64.const 0)) (i64.const 1))
(assert_return (invoke "sub" (i64.const -1) (i64.const -1)) (i64.const 0))
(assert_return (invoke "sub" (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x8000000000000000))
(assert_return (invoke "sub" (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0x7fffffffffffffff))
(assert_return (invoke "sub" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i64.const 0))
(assert_return (invoke "sub" (i64.const 0x3fffffff) (i64.const -1)) (i64.const 0x40000000))

(assert_return (invoke "mul" (i64.const 1) (i64.const 1)) (i64.const 1))
(assert_return (invoke "mul" (i64.const 1) (i64.const 0)) (i64.const 0))
(assert_return (invoke "mul" (i64.const -1) (i64.const -1)) (i64.const 1))
(assert_return (invoke "mul" (i64.const 0x1000000000000000) (i64.const 4096)) (i64.const 0))
(assert_return (invoke "mul" (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0))
(assert_return (invoke "mul" (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x8000000000000000))
(assert_return (invoke "mul" (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x8000000000000001))
(assert_return (invoke "mul" (i64.const 0x0123456789abcdef) (i64.const 0xfedcba9876543210)) (i64.const 0x2236d88fe5618cf0))
(assert_return (invoke "mul" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i64.const 1))

(assert_trap (invoke "div_s" (i64.const 1) (i64.const 0)) "integer divide by zero")
(assert_trap (invoke "div_s" (i64.const 0) (i64.const 0)) "integer divide by zero")
(assert_trap (invoke "div_s" (i64.const 0x8000000000000000) (i64.const -1)) "integer overflow")
(assert_trap (invoke "div_s" (i64.const 0x8000000000000000) (i64.const 0)) "integer divide by zero")
(assert_return (invoke "div_s" (i64.const 1) (i64.const 1)) (i64.const 1))
(assert_return (invoke "div_s" (i64.const 0) (i64.const 1)) (i64.const 0))
(assert_return (invoke "div_s" (i64.const 0) (i64.const -1)) (i64.const 0))
(assert_return (invoke "div_s" (i64.const -1) (i64.const -1)) (i64.const 1))
(assert_return (invoke "div_s" (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0xc000000000000000))
(assert_return (invoke "div_s" (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 0xffdf3b645a1cac09))
(assert_return (invoke "div_s" (i64.const 5) (i64.const 2)) (i64.const 2))
(assert_return (invoke "div_s" (i64.const -5) (i64.const 2)) (i64.const -2))
(assert_return (invoke "div_s" (i64.const 5) (i64.const -2)) (i64.const -2))
(assert_return (invoke "div_s" (i64.const -5) (i64.const -2)) (i64.const 2))
(assert_return (invoke "div_s" (i64.const 7) (i64.const 3)) (i64.const 2))
(assert_return (invoke "div_s" (i64.const -7) (i64.const 3)) (i64.const -2))
(assert_return (invoke "div_s" (i64.const 7) (i64.const -3)) (i64.const -2))
(assert_return (invoke "div_s" (i64.const -7) (i64.const -3)) (i64.const 2))
(assert_return (invoke "div_s" (i64.const 11) (i64.const 5)) (i64.const 2))
(assert_return (invoke "div_s" (i64.const 17) (i64.const 7)) (i64.const 2))

(assert_trap (invoke "div_u" (i64.const 1) (i64.const 0)) "integer divide by zero")
(assert_trap (invoke "div_u" (i64.const 0) (i64.const 0)) "integer divide by zero")
(assert_return (invoke "div_u" (i64.const 1) (i64.const 1)) (i64.const 1))
(assert_return (invoke "div_u" (i64.const 0) (i64.const 1)) (i64.const 0))
(assert_return (invoke "div_u" (i64.const -1) (i64.const -1)) (i64.const 1))
(assert_return (invoke "div_u" (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0))
(assert_return (invoke "div_u" (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0x4000000000000000))
(assert_return (invoke "div_u" (i64.const 0x8ff00ff00ff00ff0) (i64.const 0x100000001)) (i64.const 0x8ff00fef))
(assert_return (invoke "div_u" (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 0x20c49ba5e353f7))
(assert_return (invoke "div_u" (i64.const 5) (i64.const 2)) (i64.const 2))
(assert_return (invoke "div_u" (i64.const -5) (i64.const 2)) (i64.const 0x7ffffffffffffffd))
(assert_return (invoke "div_u" (i64.const 5) (i64.const -2)) (i64.const 0))
(assert_return (invoke "div_u" (i64.const -5) (i64.const -2)) (i64.const 0))
(assert_return (invoke "div_u" (i64.const 7) (i64.const 3)) (i64.const 2))
(assert_return (invoke "div_u" (i64.const 11) (i64.const 5)) (i64.const 2))
(assert_return (invoke "div_u" (i64.const 17) (i64.const 7)) (i64.const 2))

;; (assert_trap (invoke "rem_s" (i64.const 1) (i64.const 0)) "integer divide by zero")
;; (assert_trap (invoke "rem_s" (i64.const 0) (i64.const 0)) "integer divide by zero")
;; (assert_return (invoke "rem_s" (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 1) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 0) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 0) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const -1) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0))
;; (assert_return (invoke "rem_s" (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const -807))
;; (assert_return (invoke "rem_s" (i64.const 5) (i64.const 2)) (i64.const 1))
;; (assert_return (invoke "rem_s" (i64.const -5) (i64.const 2)) (i64.const -1))
;; (assert_return (invoke "rem_s" (i64.const 5) (i64.const -2)) (i64.const 1))
;; (assert_return (invoke "rem_s" (i64.const -5) (i64.const -2)) (i64.const -1))
;; (assert_return (invoke "rem_s" (i64.const 7) (i64.const 3)) (i64.const 1))
;; (assert_return (invoke "rem_s" (i64.const -7) (i64.const 3)) (i64.const -1))
;; (assert_return (invoke "rem_s" (i64.const 7) (i64.const -3)) (i64.const 1))
;; (assert_return (invoke "rem_s" (i64.const -7) (i64.const -3)) (i64.const -1))
;; (assert_return (invoke "rem_s" (i64.const 11) (i64.const 5)) (i64.const 1))
;; (assert_return (invoke "rem_s" (i64.const 17) (i64.const 7)) (i64.const 3))

;; (assert_trap (invoke "rem_u" (i64.const 1) (i64.const 0)) "integer divide by zero")
;; (assert_trap (invoke "rem_u" (i64.const 0) (i64.const 0)) "integer divide by zero")
;; (assert_return (invoke "rem_u" (i64.const 1) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "rem_u" (i64.const 0) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "rem_u" (i64.const -1) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "rem_u" (i64.const 0x8000000000000000) (i64.const -1)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "rem_u" (i64.const 0x8000000000000000) (i64.const 2)) (i64.const 0))
;; (assert_return (invoke "rem_u" (i64.const 0x8ff00ff00ff00ff0) (i64.const 0x100000001)) (i64.const 0x80000001))
;; (assert_return (invoke "rem_u" (i64.const 0x8000000000000001) (i64.const 1000)) (i64.const 809))
;; (assert_return (invoke "rem_u" (i64.const 5) (i64.const 2)) (i64.const 1))
;; (assert_return (invoke "rem_u" (i64.const -5) (i64.const 2)) (i64.const 1))
;; (assert_return (invoke "rem_u" (i64.const 5) (i64.const -2)) (i64.const 5))
;; (assert_return (invoke "rem_u" (i64.const -5) (i64.const -2)) (i64.const -5))
;; (assert_return (invoke "rem_u" (i64.const 7) (i64.const 3)) (i64.const 1))
;; (assert_return (invoke "rem_u" (i64.const 11) (i64.const 5)) (i64.const 1))
;; (assert_return (invoke "rem_u" (i64.const 17) (i64.const 7)) (i64.const 3))

;; (assert_return (invoke "and" (i64.const 1) (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "and" (i64.const 0) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "and" (i64.const 1) (i64.const 1)) (i64.const 1))
;; (assert_return (invoke "and" (i64.const 0) (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "and" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const 0))
;; (assert_return (invoke "and" (i64.const 0x7fffffffffffffff) (i64.const -1)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "and" (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0xf0f0f0f0))
;; (assert_return (invoke "and" (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0xffffffffffffffff))

;; (assert_return (invoke "or" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "or" (i64.const 0) (i64.const 1)) (i64.const 1))
;; (assert_return (invoke "or" (i64.const 1) (i64.const 1)) (i64.const 1))
;; (assert_return (invoke "or" (i64.const 0) (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "or" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const -1))
;; (assert_return (invoke "or" (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "or" (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0xffffffff))
;; (assert_return (invoke "or" (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0xffffffffffffffff))

;; (assert_return (invoke "xor" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "xor" (i64.const 0) (i64.const 1)) (i64.const 1))
;; (assert_return (invoke "xor" (i64.const 1) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "xor" (i64.const 0) (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "xor" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i64.const -1))
;; (assert_return (invoke "xor" (i64.const 0x8000000000000000) (i64.const 0)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "xor" (i64.const -1) (i64.const 0x8000000000000000)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "xor" (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "xor" (i64.const 0xf0f0ffff) (i64.const 0xfffff0f0)) (i64.const 0x0f0f0f0f))
;; (assert_return (invoke "xor" (i64.const 0xffffffffffffffff) (i64.const 0xffffffffffffffff)) (i64.const 0))

;; (assert_return (invoke "shl" (i64.const 1) (i64.const 1)) (i64.const 2))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "shl" (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0xfffffffffffffffe))
;; (assert_return (invoke "shl" (i64.const 0xffffffffffffffff) (i64.const 1)) (i64.const 0xfffffffffffffffe))
;; (assert_return (invoke "shl" (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "shl" (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const 63)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const 64)) (i64.const 1))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const 65)) (i64.const 2))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const -1)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "shl" (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0x8000000000000000))

;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const 1)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x3fffffffffffffff))
;; (assert_return (invoke "shr_s" (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0xc000000000000000))
;; (assert_return (invoke "shr_s" (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x2000000000000000))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 64)) (i64.const 1))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 65)) (i64.const 0))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0))
;; (assert_return (invoke "shr_s" (i64.const 1) (i64.const 0x8000000000000000)) (i64.const 1))
;; (assert_return (invoke "shr_s" (i64.const 0x8000000000000000) (i64.const 63)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const 64)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const 65)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const -1)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const -1))
;; (assert_return (invoke "shr_s" (i64.const -1) (i64.const 0x8000000000000000)) (i64.const -1))

;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 1)) (i64.const 0))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const 1)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "shr_u" (i64.const 0x7fffffffffffffff) (i64.const 1)) (i64.const 0x3fffffffffffffff))
;; (assert_return (invoke "shr_u" (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 0x4000000000000000))
;; (assert_return (invoke "shr_u" (i64.const 0x4000000000000000) (i64.const 1)) (i64.const 0x2000000000000000))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 64)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 65)) (i64.const 0))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const -1)) (i64.const 0))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 0x7fffffffffffffff)) (i64.const 0))
;; (assert_return (invoke "shr_u" (i64.const 1) (i64.const 0x8000000000000000)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const 0x8000000000000000) (i64.const 63)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const 64)) (i64.const -1))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const 65)) (i64.const 0x7fffffffffffffff))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const -1)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const 0x7fffffffffffffff)) (i64.const 1))
;; (assert_return (invoke "shr_u" (i64.const -1) (i64.const 0x8000000000000000)) (i64.const -1))

;; (assert_return (invoke "rotl" (i64.const 1) (i64.const 1)) (i64.const 2))
;; (assert_return (invoke "rotl" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "rotl" (i64.const -1) (i64.const 1)) (i64.const -1))
;; (assert_return (invoke "rotl" (i64.const 1) (i64.const 64)) (i64.const 1))
;; (assert_return (invoke "rotl" (i64.const 0xabcd987602468ace) (i64.const 1)) (i64.const 0x579b30ec048d159d))
;; (assert_return (invoke "rotl" (i64.const 0xfe000000dc000000) (i64.const 4)) (i64.const 0xe000000dc000000f))
;; (assert_return (invoke "rotl" (i64.const 0xabcd1234ef567809) (i64.const 53)) (i64.const 0x013579a2469deacf))
;; (assert_return (invoke "rotl" (i64.const 0xabd1234ef567809c) (i64.const 63)) (i64.const 0x55e891a77ab3c04e))
;; (assert_return (invoke "rotl" (i64.const 0xabcd1234ef567809) (i64.const 0xf5)) (i64.const 0x013579a2469deacf))
;; (assert_return (invoke "rotl" (i64.const 0xabcd7294ef567809) (i64.const 0xffffffffffffffed)) (i64.const 0xcf013579ae529dea))
;; (assert_return (invoke "rotl" (i64.const 0xabd1234ef567809c) (i64.const 0x800000000000003f)) (i64.const 0x55e891a77ab3c04e))
;; (assert_return (invoke "rotl" (i64.const 1) (i64.const 63)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "rotl" (i64.const 0x8000000000000000) (i64.const 1)) (i64.const 1))

;; (assert_return (invoke "rotr" (i64.const 1) (i64.const 1)) (i64.const 0x8000000000000000))
;; (assert_return (invoke "rotr" (i64.const 1) (i64.const 0)) (i64.const 1))
;; (assert_return (invoke "rotr" (i64.const -1) (i64.const 1)) (i64.const -1))
;; (assert_return (invoke "rotr" (i64.const 1) (i64.const 64)) (i64.const 1))
;; (assert_return (invoke "rotr" (i64.const 0xabcd987602468ace) (i64.const 1)) (i64.const 0x55e6cc3b01234567))
;; (assert_return (invoke "rotr" (i64.const 0xfe000000dc000000) (i64.const 4)) (i64.const 0x0fe000000dc00000))
;; (assert_return (invoke "rotr" (i64.const 0xabcd1234ef567809) (i64.const 53)) (i64.const 0x6891a77ab3c04d5e))
;; (assert_return (invoke "rotr" (i64.const 0xabd1234ef567809c) (i64.const 63)) (i64.const 0x57a2469deacf0139))
;; (assert_return (invoke "rotr" (i64.const 0xabcd1234ef567809) (i64.const 0xf5)) (i64.const 0x6891a77ab3c04d5e))
;; (assert_return (invoke "rotr" (i64.const 0xabcd7294ef567809) (i64.const 0xffffffffffffffed)) (i64.const 0x94a77ab3c04d5e6b))
;; (assert_return (invoke "rotr" (i64.const 0xabd1234ef567809c) (i64.const 0x800000000000003f)) (i64.const 0x57a2469deacf0139))
;; (assert_return (invoke "rotr" (i64.const 1) (i64.const 63)) (i64.const 2))
;; (assert_return (invoke "rotr" (i64.const 0x8000000000000000) (i64.const 63)) (i64.const 1))

(assert_return (invoke "clz" (i64.const 0xffffffffffffffff)) (i64.const 0))
(assert_return (invoke "clz" (i64.const 0)) (i64.const 64))
(assert_return (invoke "clz" (i64.const 0x00008000)) (i64.const 48))
(assert_return (invoke "clz" (i64.const 0xff)) (i64.const 56))
(assert_return (invoke "clz" (i64.const 0x8000000000000000)) (i64.const 0))
(assert_return (invoke "clz" (i64.const 1)) (i64.const 63))
(assert_return (invoke "clz" (i64.const 2)) (i64.const 62))
(assert_return (invoke "clz" (i64.const 0x7fffffffffffffff)) (i64.const 1))

(assert_return (invoke "ctz" (i64.const -1)) (i64.const 0))
(assert_return (invoke "ctz" (i64.const 0)) (i64.const 64))
(assert_return (invoke "ctz" (i64.const 0x00008000)) (i64.const 15))
(assert_return (invoke "ctz" (i64.const 0x00010000)) (i64.const 16))
(assert_return (invoke "ctz" (i64.const 0x8000000000000000)) (i64.const 63))
(assert_return (invoke "ctz" (i64.const 0x7fffffffffffffff)) (i64.const 0))

;; (assert_return (invoke "popcnt" (i64.const -1)) (i64.const 64))
;; (assert_return (invoke "popcnt" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "popcnt" (i64.const 0x00008000)) (i64.const 1))
;; (assert_return (invoke "popcnt" (i64.const 0x8000800080008000)) (i64.const 4))
;; (assert_return (invoke "popcnt" (i64.const 0x7fffffffffffffff)) (i64.const 63))
;; (assert_return (invoke "popcnt" (i64.const 0xAAAAAAAA55555555)) (i64.const 32))
;; (assert_return (invoke "popcnt" (i64.const 0x99999999AAAAAAAA)) (i64.const 32))
;; (assert_return (invoke "popcnt" (i64.const 0xDEADBEEFDEADBEEF)) (i64.const 48))

;; (assert_return (invoke "extend8_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend8_s" (i64.const 0x7f)) (i64.const 127))
;; (assert_return (invoke "extend8_s" (i64.const 0x80)) (i64.const -128))
;; (assert_return (invoke "extend8_s" (i64.const 0xff)) (i64.const -1))
;; (assert_return (invoke "extend8_s" (i64.const 0x01234567_89abcd_00)) (i64.const 0))
;; (assert_return (invoke "extend8_s" (i64.const 0xfedcba98_765432_80)) (i64.const -0x80))
;; (assert_return (invoke "extend8_s" (i64.const -1)) (i64.const -1))

;; (assert_return (invoke "extend16_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend16_s" (i64.const 0x7fff)) (i64.const 32767))
;; (assert_return (invoke "extend16_s" (i64.const 0x8000)) (i64.const -32768))
;; (assert_return (invoke "extend16_s" (i64.const 0xffff)) (i64.const -1))
;; (assert_return (invoke "extend16_s" (i64.const 0x12345678_9abc_0000)) (i64.const 0))
;; (assert_return (invoke "extend16_s" (i64.const 0xfedcba98_7654_8000)) (i64.const -0x8000))
;; (assert_return (invoke "extend16_s" (i64.const -1)) (i64.const -1))

;; (assert_return (invoke "extend32_s" (i64.const 0)) (i64.const 0))
;; (assert_return (invoke "extend32_s" (i64.const 0x7fff)) (i64.const 32767))
;; (assert_return (invoke "extend32_s" (i64.const 0x8000)) (i64.const 32768))
;; (assert_return (invoke "extend32_s" (i64.const 0xffff)) (i64.const 65535))
;; (assert_return (invoke "extend32_s" (i64.const 0x7fffffff)) (i64.const 0x7fffffff))
;; (assert_return (invoke "extend32_s" (i64.const 0x80000000)) (i64.const -0x80000000))
;; (assert_return (invoke "extend32_s" (i64.const 0xffffffff)) (i64.const -1))
;; (assert_return (invoke "extend32_s" (i64.const 0x01234567_00000000)) (i64.const 0))
;; (assert_return (invoke "extend32_s" (i64.const 0xfedcba98_80000000)) (i64.const -0x80000000))
;; (assert_return (invoke "extend32_s" (i64.const -1)) (i64.const -1))

(assert_return (invoke "eqz" (i64.const 0)) (i32.const 1))
(assert_return (invoke "eqz" (i64.const 1)) (i32.const 0))
(assert_return (invoke "eqz" (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "eqz" (i64.const 0x7fffffffffffffff)) (i32.const 0))
(assert_return (invoke "eqz" (i64.const 0xffffffffffffffff)) (i32.const 0))

(assert_return (invoke "eq" (i64.const 0) (i64.const 0)) (i32.const 1))
(assert_return (invoke "eq" (i64.const 1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "eq" (i64.const -1) (i64.const 1)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "eq" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "eq" (i64.const -1) (i64.const -1)) (i32.const 1))
(assert_return (invoke "eq" (i64.const 1) (i64.const 0)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0) (i64.const 1)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0))
(assert_return (invoke "eq" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0))
(assert_return (invoke "eq" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0))

(assert_return (invoke "ne" (i64.const 0) (i64.const 0)) (i32.const 0))
(assert_return (invoke "ne" (i64.const 1) (i64.const 1)) (i32.const 0))
(assert_return (invoke "ne" (i64.const -1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "ne" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0))
(assert_return (invoke "ne" (i64.const -1) (i64.const -1)) (i32.const 0))
(assert_return (invoke "ne" (i64.const 1) (i64.const 0)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0) (i64.const 1)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1))
(assert_return (invoke "ne" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "ne" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1))

;; (assert_return (invoke "lt_s" (i64.const 0) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const -1) (i64.const 1)) (i32.const 1))
;; (assert_return (invoke "lt_s" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const -1) (i64.const -1)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 1) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 0) (i64.const 1)) (i32.const 1))
;; (assert_return (invoke "lt_s" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1))
;; (assert_return (invoke "lt_s" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1))
;; (assert_return (invoke "lt_s" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "lt_s" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1))
;; (assert_return (invoke "lt_s" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0))

;; (assert_return (invoke "lt_u" (i64.const 0) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const -1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const -1) (i64.const -1)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 1) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0) (i64.const 1)) (i32.const 1))
;; (assert_return (invoke "lt_u" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "lt_u" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1))
;; (assert_return (invoke "lt_u" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "lt_u" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1))

(assert_return (invoke "le_s" (i64.const 0) (i64.const 0)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const -1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const -1) (i64.const -1)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 1) (i64.const 0)) (i32.const 0))
(assert_return (invoke "le_s" (i64.const 0) (i64.const 1)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "le_s" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "le_s" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "le_s" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0))

(assert_return (invoke "le_u" (i64.const 0) (i64.const 0)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const 1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const -1) (i64.const 1)) (i32.const 0))
(assert_return (invoke "le_u" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const -1) (i64.const -1)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const 1) (i64.const 0)) (i32.const 0))
(assert_return (invoke "le_u" (i64.const 0) (i64.const 1)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0))
(assert_return (invoke "le_u" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 1))
(assert_return (invoke "le_u" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "le_u" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0))
(assert_return (invoke "le_u" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1))

;; (assert_return (invoke "gt_s" (i64.const 0) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const -1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const -1) (i64.const -1)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 1) (i64.const 0)) (i32.const 1))
;; (assert_return (invoke "gt_s" (i64.const 0) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "gt_s" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "gt_s" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "gt_s" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1))

(assert_return (invoke "gt_u" (i64.const 0) (i64.const 0)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const 1) (i64.const 1)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const -1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "gt_u" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const -1) (i64.const -1)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const 1) (i64.const 0)) (i32.const 1))
(assert_return (invoke "gt_u" (i64.const 0) (i64.const 1)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1))
(assert_return (invoke "gt_u" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0))
(assert_return (invoke "gt_u" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "gt_u" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "gt_u" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0))

;; (assert_return (invoke "ge_s" (i64.const 0) (i64.const 0)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 1) (i64.const 1)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const -1) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "ge_s" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const -1) (i64.const -1)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 1) (i64.const 0)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 0) (i64.const 1)) (i32.const 0))
;; (assert_return (invoke "ge_s" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 0))
;; (assert_return (invoke "ge_s" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0))
;; (assert_return (invoke "ge_s" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1))
;; (assert_return (invoke "ge_s" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 0))
;; (assert_return (invoke "ge_s" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 1))

(assert_return (invoke "ge_u" (i64.const 0) (i64.const 0)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const -1) (i64.const 1)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0x8000000000000000) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0x7fffffffffffffff) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const -1) (i64.const -1)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 1) (i64.const 0)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0) (i64.const 1)) (i32.const 0))
(assert_return (invoke "ge_u" (i64.const 0x8000000000000000) (i64.const 0)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0) (i64.const 0x8000000000000000)) (i32.const 0))
(assert_return (invoke "ge_u" (i64.const 0x8000000000000000) (i64.const -1)) (i32.const 0))
(assert_return (invoke "ge_u" (i64.const -1) (i64.const 0x8000000000000000)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0x8000000000000000) (i64.const 0x7fffffffffffffff)) (i32.const 1))
(assert_return (invoke "ge_u" (i64.const 0x7fffffffffffffff) (i64.const 0x8000000000000000)) (i32.const 0))