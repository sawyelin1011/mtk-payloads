.syntax unified

.code 32

.global start
.section .text.start
start:
    add r3, pc, #1  // DA seem to think we're in ARM mode, so we need to switch to thumb
    bx r3           // Now we jump to main!
