(module
    (type $f0 (func (param i32)(result i32)))
    (func $fibo (export "fibo") (type $f0)(param $n i32)(result i32)
        local.get $n
        i32.const 0
        i32.eq
        if
            i32.const 0
            return
        end

        local.get $n
        i32.const 1
        i32.eq
        if
            i32.const 1
            return
        end

        local.get $n
        i32.const 1
        i32.sub
        call $fibo
        local.get $n
        i32.const 2
        i32.sub
        call $fibo
        
        i32.add
        return
    )
)