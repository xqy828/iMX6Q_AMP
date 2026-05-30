    .code 32
    .section ".text","ax"
    .align 2
.extern softuart
.global early_debug
.func early_debug
early_debug:
    push {r4,r5, lr}
    ldr r4,=softuart
    mov r5,#0x0
    str r5, [r4]
    str r5, [r4, #8]
    dsb
    ldr r5,=0xdeadbeef
    str r5, [r4, #8]
    mov r5,#0x2
    str r5,[r4]
    pop {r4,r5, pc}
.endfunc
