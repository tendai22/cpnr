#! /bin/sh
d=`basename -s s "$1"`
( cat <<"EOF"
    .macro m_jmp dest
    .dw  dest - .
    .endm
EOF
awk '
@include "opcode.inc"
BEGIN {
    for (sym in opcode) {
        printf("    .macro %-12s\n", sym)
        printf("    .dw  0h%04x    ; %s\n", opcode[sym], sym)
        printf("    .endm\n")
    }
}' /dev/null
cat "$1" |
awk '
$1 == ".head" {
    printf ("    .db  %d\n", length($2) - 2)
    printf ("    .ascii   %s\n", $2)
    next
}
{   print }
' ) > "${d}asm"
