#! /bin/sh
flag="$1"
shift
# source code, add ORG/USER_ORG lines here
(   echo "        .org      $1"
    echo "        .user_org $2"
    shift; shift;
    cat "$@" )|tee xx |
awk '
BEGIN {
    mode = "'$flag'"
    if (mode == "-h") {
        print "extern word_t org_addr;"
        print "extern word_t user_org_addr;"
    }
}
$1 == ".org" {
    horg = haddr = strtonum($2)
    #if (mode == "-h")
    #    printf("#define %-16s 0x%04x\n", "ORG_ADDR", haddr)
    if (mode == "-f")
        printf(": %-16s 0x%04x ;\n", "ORG_CONST", haddr)
}
$1 == ".user_org" {
    org = addr = strtonum($2)
    #if (mode == "-h")
    #    printf("#define %-16s 0x%04x\n", "USER_ORG_ADDR", addr)
    if (mode == "-f")
        printf(": %-16s 0x%04x ;\n", "USER_ORG_CONST", addr)
    next
}
$1 == ".head" || $1 == ".user" || $1 == ".const" || $1 == ".huser" {
    name = $2
    if ($2 ~ /^[A-Z][A-Z_0-9]*$/) {
        name = $2 "_ADDR"
        cname = $2 "_CONST"
        hname = $2 "_HEAD"
    } else if ($1 == ".head") {
        hname = $2
        cname = name = ""
    } else {
        name = $2
        cname = hname = ""
    }
}
$1 == ".head" || $1 == ".const" || $1 == ".user" || $1 == ".huser" {
    if (mode == "-h" && name ~ /^[A-Z][A-Z0-9_]*$/) {
        printf("#define %-16s (%s + 0x%04x)\n", hname, "org_addr", haddr - horg);
        if ($1 == ".user")
            printf("#define %-16s (%s + 0x%04x)\n", name, "user_org_addr", addr - org);       
    }
    if (mode == "-s") {
        expr = ""
        if ($3 != "" && $3 !~ /^\/\//)
            expr = $3
        if (expr ~ /^ *$/)
            expr = "0"
        printf("    .dw   %-12s ; xx %04x %s\n", expr, haddr, (hname != "") ? hname : name);
    }
    if (mode == "-f") {
        if (hname != "")
            printf(": %-16s 0x%04x ;\n", hname, haddr);
        if ($3 == "//") {
            code = $4 " " $5 " " $6 " " $7 " " $8 " " $9
            if ($3 != "//")
                code = $3 " " code
            if (hname != "")
                printf("%-16s %s swap !\n", hname, code);
        }
        if ($1 == ".user") {
            printf(": %-16s 0x%04x ; \\ xx\n", name, addr);
            if ($3 == "//") {
                code = $4 " " $5 " " $6 " " $7 " " $8 " " $9
                if ($3 != "//")
                    code = $3 " " code
                printf("%-16s %s swap !\n", name, code)
            }
        }
        if ($1 == ".huser") {
            printf(": %-16s 0x%04x ;\n", name, addr) > "huser.f"
        }
    }
    if ($1 == ".user" || $1 == ".huser") {
        addr += 2
        haddr += 2
    }
    if ($1 == ".head" || $1 == ".const")
        haddr += 2;
    next
}
mode == "-s" {
    print
}
'
