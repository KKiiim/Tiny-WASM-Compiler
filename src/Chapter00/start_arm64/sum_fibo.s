.text
.global mySum myFibo

mySum:
    # x0 is input n
    # x1 store loop i
    # x2 store sum result
    mov x1, #0
    mov x2, #0
inner_loop:
    add x2, x1, x2
    add x1, x1, #1
    cmp x1, x0
    ble inner_loop

    mov x0, x2
    ret

myFibo:
    # x0 is input n, caller stored

    # ignore use register for func-call optimize, just store in stack

    # store caller's sp, lr
    stp x29, x30, [sp, -32]!    # sp = sp -32, store 4 8B number: x29(sp), x30(lr), x19(tmp), x0
    mov x29, sp # x29 store the current stack pointer
    stp x19, x0, [sp, 16] # x19, x0 stored for this stackFrame use 

    cmp x0, 0
    beq prepare_ret
    cmp x0, 1
    beq prepare_ret

    sub x0, x0, #1
    bl myFibo # bl store returnAddress in x30
    # x0 is ret
    str x0, [sp, 16] # store fibo(n-1) in x19

    ldr x0, [sp, 24]
    sub x0, x0, #2
    bl myFibo
    # x0 is fibo(n-2)
    ldr x19, [sp, 16]
    add x0, x0, x19 # x0 = fibo(n-1) + fibo(n-2)

prepare_ret:
    # ldr x19, [sp, 16]
    # do not change x0, x0 is set before prepare_ret
    # x19 just for tmp, no need for reset
    ldp x29, x30, [sp], 32 # sp += 32
    ret