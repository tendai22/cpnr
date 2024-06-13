#! /bin/sh
flag="$1"
shift
# source code, add ORG/USER_ORG lines here
( case "$flag" in
  '-f')
    echo "    .const ORG      // $1"
    echo "    .const USER_ORG // $2"
    shift
    shift
    ;;
  '-s'|'-h')
    echo "        .org      $1"
    echo "        .user_org $2"
    shift
    shift
    ;;
  esac
  cat "$@" )|
case "$flag" in
'-h')
    awk '
    $1 == ".org" {
        haddr = strtonum($2)
        printf("#define %-16s 0x%04x\n", "ORG_ADDR", haddr)
    }
    $1 == ".user_org" {
        addr = strtonum($2)
        printf("#define %-16s 0x%04x\n", "USER_ORG_ADDR", addr)
    }
    $1 == ".user" {
        if ($2 ~ /^[A-Z][A-Z0-9]*$/) {
            name = $2 "_ADDR"
            printf("#define %-16s 0x%04x\n", name, addr);
        }
        addr += 2;
        next
    }
    $1 == ".head" {
        if ($2 ~ /^[A-Z][A-Z0-9]*$/) {
            name = $2 "_ADDR"
            printf("#define %-16s 0x%04x\n", name, haddr);
        }
        haddr += 2;
        next
    }
    '
    ;;
'-f')
    awk '
    $1 == ".const" && $2 == "ORG" { haddr = strtonum($4) }
    $1 == ".const" && $2 == "USER_ORG" { addr = strtonum($4) }
    $1 == ".user" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/)
            name = $2 "_ADDR"
        printf(": %-16s 0x%04x ;\n", name, addr);
        if ($3 == "//") {
            code = $4 " " $5 " " $6 " " $7 " " $8 " " $9
            if ($3 != "//")
                code = $3 " " code
            printf("%-16s %s swap !\n", name, code)
        }
        addr += 2;
    }
    $1 == ".head" || $1 == ".const" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z_0-9]*$/) {
            name = $2 "_ADDR"
            cname = $2 "_CONST"
        }
        if ($1 == ".head") {
            printf(": %-16s 0x%04x ;\n", name, haddr);
            haddr += 2;
        }
        if ($1 == ".const") {
            code = $4 " " $5 " " $6 " " $7 " " $8 " " $9
            if ($3 != "//")
                code = $3 " " code
           printf(": %-16s %s ;\n", cname, code);
        }
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
        haddr = strtonum($2)
        next
    }
    $1 == ".user_org" {
        addr = strtonum($2)
        next
    }
    $1 == ".head" {
        name = $2
        if ($2 ~ /^[A-Z][A-Z0-9]*$/)
            name = $2 "_ADDR"
        expr = "0"
        if ($3 != "")
            expr = $3
        if (expr ~ /^ *$/)
            expr = "0"
        printf("    .dw   %-12s ; %04x %s\n", expr, haddr, name);
        haddr += 2;
        next
    }
    $1 != ".user" && $1 != ".const" { print }
    '
    ;;
*)
    echo "usage $0 [-h|-f|-s] org u-org user.def" >&2
    exit 2
    ;;
esac
