(module
    (type $f0 (func (param i32)(result i32)))
    (func $sum (export "sum") (type $f0) (param $p0 i32)(result i32)
        (local $i i32)(local $ret i32)
        i32.const 0
        local.set $i   
        i32.const 0
        local.set $ret

        loop $l0
            local.get $i
            local.get $ret
            i32.add
            local.set $ret

            local.get $i
            i32.const 1
            i32.add
            local.set $i

            local.get $i
            local.get $p0
            i32.ne
            br_if $l0
        end $l0

        local.get $ret
        return
    )
)