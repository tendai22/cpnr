#! /bin/sh
cat "$@" |
awk '
$1 == ".org" { addr = strtonum($2) }
$1 == ".utop" { up_addr = strtonum($2) }
$1 == ".user" {
    name = $2
    if ($2 ~ /^[A-Z][A-Z0-9]*$/)
        name = $2 "_ADDR"
    printf("    .equ %s, 0x%04x\n", name, `up_addr);
    up_addr += 2;
    expr = $3 " " $4 " " $5 " " $6 " " $7
    if (expr ~ /^  *$/)
        expr = "0";
    if (name != "END_ADDR")
        printf("             .dw  %s\n", expr);
    addr += 2;
}
'
