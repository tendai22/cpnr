#! /bin/sh
flag="$1"
shift
cat "$@" |
case "$flag" in
'-h')
    awk '
    $1 == ".user_org" { addr = strtonum($2) }
    $1 == ".user" || $1 == ".const" {
        if ($2 ~ /^[A-Z][A-Z0-9]*$/) {
            name = $2 "_ADDR"
            printf("#define %-16s 0x%04x\n", name, addr);
        }
        addr += 2;
    }
    '
    ;;
'-f')
    awk '
    $1 == ".org" { print ": ORG_ADDR " $2 " ;" }
    $1 == ".user_org" { addr = strtonum($2) }
    $1 == ".user" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/)
            name = $2 "_ADDR"
        init_code = ""
        if ($3 == "//")
            init_code = $4 " " $5 " " $6 " " $7 " " $8 " " $9
        printf(": %-16s 0x%04x ;\n", name, addr);
        if (init_code != "")
            body = body " " sprintf("%-10s %-16s !\n", init_code, name);
        addr += 2;
    }
    $1 == ".const" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/) {
            name = $2 "_ADDR"
            cname = $2 "_CONST"
        }
        printf(": %-16s 0x%04x ;\n", name, addr);
        code = $3 " " $4 " " $5 " " $6 " " $7 " " $8 " " $9
        if (code != "")
            printf(": %-10s %-16s ;\n", cname, code);
        addr += 2;
    }
    END {
        if (body != "")
            print body
    #   print ": init_user" > "upost.f"
    #   print body > "upost.f"
    #   print "  ;" > "upost.f"
        print "variable dumpuser END_ADDR DICTTOP_ADDR - cells + ( 0x41 .ps ) allot" > "upost.f"
        print ": restore_user dumpuser DICTTOP_ADDR END_ADDR DICTTOP_ADDR - cmove ;" > "upost.f"
        print ": cold restore_user abort ;" > "upost.f"
        print "'\'' cold COLD_ADDR !" > "upost.f"
        print "DICTTOP_ADDR dumpuser END_ADDR DICTTOP_ADDR - cmove" > "upost.f"
        print "cold" > "upost.f"
    }
    '
    ;;

'-s')
    sed 's/\/\/.*$//' |
    awk '
    $1 == ".org" { 
        print   # print it
        next
    }
    $1 == ".user_org" {
        addr = strtonum($2)
        next
    }
    $1 == ".user" || $1 == ".const" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/)
            name = $2 "_ADDR"
        expr = "0"
        if ($1 == ".user")
            expr = $3 " " $4 " " $5 " " $6 " " $7
        if (expr ~ /^ *$/)
            expr = "0"
        printf("    .dw   %s    ; %s\n", expr, name);
        addr += 2;
        next
    }
    $1 != "" { print }
    '
    ;;
*)
    echo "usage $0 [-h|-f|-s] user.def" >&2
    exit 2
    ;;
esac
