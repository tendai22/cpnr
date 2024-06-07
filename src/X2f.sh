#! /bin/sh
cat "$@" |
sed 's/  */\
/g
' |
awk '
$1 ~ /^=/ {
    s = "0x" substr($1,2)
    addr = strtonum(s)
}
$1 ~ /^[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]$/ {
    v = strtonum("0x" $1)
    printf "0x%04x 0x%04x !\n", v, addr
    addr += 2
    next
}
$1 ~ /^[0-9A-Fa-f][0-9A-Fa-f]$/ {
    v = strtonum("0x" $1)
    printf "0x%02x 0x%04x c!\n", v, addr
    addr += 1
    next
}
'