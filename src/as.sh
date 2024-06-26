#! /bin/sh
for f
do  case "$f" in
    *.s)  ;;
    *)  echo "$2: usage $0 files.s ..." >&2; exit 2;;
    esac
    ( cat "$f"; echo "PASS2"; cat "$f") |
    sed '#
    /^[^: ][^: ]*:/{
        s/^\([^: ][^: ]*\):.*$/Label: \1/
        b
    }
    # comment out
    /^#/d
    /^\/\//d
    s/\/\*.*\*\// /
    ' | #cat ; exit
    awk '#
    @include "opcode.inc"
    function expand(k,n,v){
        n = k
        while (n !~ /^[0-9]+/ && n !~ /^$/) {
            v = label[n]
            #print "n:" n ",v:" v
            n = v
        }
        if (label[k] > 0) {
            #print "k:" k ",n:" n
            #label[k] = n
        }
        #print "-> label[" k "] = " label[k]
        return n
    }
    function eval(s, c, n){
        gsub(/\./, pc, s)
	    c = "awk '\''BEGIN{print " s "}'\''"
	    c | getline n
	    close(c)
	    return n
    }
    function ascii(s, n){
        n = index(" !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", (s ""))
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
            s = substr(str, 1, 2)
            if (first) {
                ostr = sprintf("%04X %s   %s", pc, s, code)
                first = 0
            } else {
                ostr = sprintf("%04X %s", pc, s)
            }
            if (pass2)
                print ostr
            pc += length(s) / 2
            str = substr(str, 3)
        }
        return pc
    }
    function operand(str,n){
        if (str ~ /^[0-9][0-9]*$/) 
            n = strtonum(str)
        else if (str ~ /^0[xX][0-9A-Fa-f][0-9A-Fa-f]*$/)
            n = strtonum(str)
        else {
            n = label[str]
            if (n ~ /^$/) {
                n = eval(str)
            } else {
                # macro expansion
                if (n !~ /^[0-9]+/ && n !~ /^$/) {
                    n = expand(str)
                }
                if (n > 0) {
                    #print "label[" str "] = " n "," label[str]
                }
            }
        }
        # print "operand: n = " n
        return n
    }
    # main actions
    BEGIN {
        stderr = "/dev/tty"
    }
    /^PASS2/{
        print $0
        pass2 = 1
        pc = org
        linebase = NR
        end_flag = 0;
        next
    }
    end_flag == 1{
        next
    }
    $1 == "Label:" && $2 ~ /^[A-Za-z_][A-Za-z0-9]+/ {
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
    $1 ~ /^\.section/ {
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
            } else if ($3 > 32) {
                n = n + $3
            }
        }
        hex = packit(str)
        hex = sprintf("%02x%s", n, hex)
        pc = outit(pc, hex, pass2, $0)
        next
    }
    $1 ~ /^\.dw/ {
        n = operand($2)
        if (pass2) {
            if (n ~ /^$/) {
                print "ERROR: " NR - linebase ": label " $2 ", not defined" >> stderr
                next
            }
            printf "%04X %04X %s\n", pc, n, $0
        }
        pc += 2
        next
    }
    $1 ~ /^\.equ/ {
        gsub(/,$/,"",$2)
        gsub(/,$/,"",$3)
        label[$2] = $3
        next
    }
    $1 ~ /^\.end *$/{
        end_flag = 1
        print "              end"
        next
    }
    /^ *$/{
        print
        next
    }
    $1 == "m_jmp" {
        #print "m_jmp $2 = " $2
        n = operand($2)
        if (pass2) {
            v = n - pc - 2;
            #print "v = " v
            if (v <= -16384 || v >= 16384) {
                # print "ERROR: " NR - linebase ": " $2 " jmp operand out of range" >> stderr
                next
            }
            if (v >= 0)
                v += 32768
            else
                v += 65536
            printf "%04X %04X %s\n", pc, v, $0
        }
        pc += 2
        next
    }
    {
        if ($1 == "Label:")
            next 
        if (opcode[$1] ~ /^$/) {
            print "ERROR: " NR - linebase ": opcode " $0 ", not defined" >> stderr
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
        print label["entry_head"]
        for (k in label) {
            printf "%-20s %04X\n", k ":", expand(k) | "sort"
        }
    }
    ' |
    sed '1,/^PASS2/d'
done
