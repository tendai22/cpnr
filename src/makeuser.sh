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
        expr = "[ " $3 " " $4 " " $5 " " $6 " " $7 " ]"
        body = body " " sprintf("%-10s literal %-16s !\n", expr, name);
        printf(": %-16s 0x%04x ;\n", name, addr, expr, name);
        addr += 2;
    }
    # END {
    #    print ": init_user" > "upost.f"
    #    print body > "upost.f"
    #    print "  ;" > "upost.f"
    # }
    '
    ;;
*)
    echo "usage $0 [-h|-f] user.def" >&2
    ;;
esac
