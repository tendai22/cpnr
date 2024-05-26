#! /bin/sh
flag="$1"
shift
cat "$@" |
case "$flag" in
'-h')
    awk '
    $1 == ".org" { addr = strtonum($2) }
    $1 == ".user" && $2 ~ /^[A-Z][A-Z0-9]*$/ {
        name = $2 "_ADDR"
        printf("#define %-16s 0x%04x\n", name, addr);
        addr += 2;
    }
    '
    ;;
'-f')
    awk '
    $1 == ".org" { addr = strtonum($2) }
    $1 == ".user" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/)
            name = $2 "_ADDR"
        printf(": %-16s 0x%04x ;\n", name, addr);
        addr += 2;
    }
    '
    ;;
*)
    echo "usage $0 [-h|-f] user.def" >&2
    ;;
esac
