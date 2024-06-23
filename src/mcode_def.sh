#! /bin/sh
awk '
@include "opcode.inc"
BEGIN {
    for (sym in opcode) {
        printf("    .macro %-12s\n", sym)
        printf("    .dw  0h%04x    ; %s\n", opcode[sym], sym)
        printf("    .endm\n")
    }
}' /dev/null
