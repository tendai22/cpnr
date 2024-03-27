#! /bin/sh
for f
do  case "$f" in
    '*.s')  ;;
    *)  echo "usage $0 files.s ..." >&2;;
    esac
    ( cat "$f"; echo "PASS2"; cat "$f") |
    sed '#
    /^[^: ][^: ]*:/{
        s/^\([^: ][^: ]*\):.*$/Label: \1/
        b
    }' |
    awk '#
    @include "opcode.inc"
    function eval(s, c, n){
        gsub(/\./, pc, s)
	    c = "awk '\''BEGIN{print " s "}'\''"
	    c | getline n
	    close(c)
	    return n
    }
    function ascii(s, n){
        n = index(" !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvqxyz{|}~", (s ""))
        return n > 0 ? n + 31 : n
    }
    function hex2dec(s){
        if (s !~ /^0x/)
            s = "0x" s
        return strtonum(s) 
    }
    function packit(s, c, n, d){
        d = ""
        while(length(s) > 0) {
            c = substr(s, 1, 1)
            n = ascii(c)
            s = substr(s, 2)
            d = d sprintf("%02X", n)
        }
        return d
    }
    function outit(pc, str, pass2, code, n, m, s, first){
        first = 1
        while (length(str) > 0) {
            s = substr(str, 1, 4)
            if (first) {
                ostr = sprintf("%04X %s %s", pc, s, code)
                first = 0
            } else {
                ostr = sprintf("%04X %s", pc, s)
            }
            if (pass2)
                print ostr
            pc += length(s) / 2
            str = substr(str, 5)
        }
        return pc
    }
    # main actions
    #BEGIN {
    #    opcode["m_colon"]   = 0xc001
    #    opcode["m_next"]    = 0xc002    
    #    opcode["m_run"]     = 0xc003
    #    opcode["m_semi"]    = 0xc004
    #    opcode["m_bnz"]     = 0xc005
    #    opcode["m_bne"]     = 0xc005
    #    opcode["m_bra"]     = 0xc006
    #    pass2 = 0
    #}
    /^PASS2/{
        print $0
        pass2 = 1
        pc = org
        linebase = NR
        # resolve equ reference
        for (k in label) {
            s = label[k]
            while (s !~ /^[0-9][0-9]*/ && s !~ /^$/) {
                print s
                s = label[s]
            }
            if (s ~ /^$/) {
                continue
            }
            v = s
            s = k
            s = label[k]
            while (s !~ /^[0-9][0-9]*/ && s !~ /^$/) {
                print "s:" s
                s0 = s
                s = label[s]
                label[s0] = v
                print "label[" s0 "] = " sprintf("%04X", v)
            }
        }
    }
    $1 ~ /^Label:/ {
        label[$2] = pc
        next
    }
    $1 ~ /^.org/ {
        pc = hex2dec($2)
        org = pc
        next
    }
    $1 ~ /^\.global/ {
        next
    }
    $1 ~ /^.equ/ {
        s = gensub(/,$/,"", "g", $2)
        v= gensub(/,$/,"", "g", $3)
        label[s] = v
        print ".equ " s " " v
        next
    }
    $1 ~ /^.head/ {
        str = gensub(/"([^"]+)"/,"\\1", "g", $2)
        n = length(str)
        if ((n % 2) == 0)
            str = str " "
        else
            str = str "  "
        if (NF >= 3) {
            if ($3 ~ /immediate/) {
                n = n + 128
            }
        }
        hex = packit(str)
        hex = sprintf("%02x%s", n, hex)
        pc = outit(pc, hex, pass2, $0)
        next
    }
    $1 ~ /^\.dw/ {
        if ($2 ~ /^[0-9][0-9]*$/) 
            n = strtonum($2)
        else if ($2 ~ /^0[xX][0-9A-Fa-f][0-9A-Fa-f]*$/)
            n = strtonum($2)
        else {
            n = label[$2]
            if (n ~ /^$/) {
                n = eval($2)
            }
        }
        if (pass2) {
            if (n ~ /^$/) {
                print "ERROR: " NR - linebase ": label " $2 ", not defined"
                next
            }
            printf "%04X %04X %s\n", pc, n, $0
        }
        pc += 2
        next
    }
    /^ *$/{
        print
        next
    }
    { 
        if (opcode[$1] ~ /^$/) {
            print "ERROR: " NR - linebase ": opcode " $0 ", not defined"
            next
        }
        s = opcode[$1]
        n = strtonum(s)
        if (pass2) {
            printf "%04X %04X %s\n", pc, n, $0
        }
        pc += 2
        next
    }
    END {
        for (k in label) {
            printf "%s: %04X\n", k, label[k]
        }
    }
    '
done
