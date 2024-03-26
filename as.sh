#! /bin/sh
for f
do  case "$f" in
    '*.s')  ;;
    *)  echo "usage $0 files.s ..." >&2;;
    esac
    cat "$f" |
    sed '#
    /^[^: ][: ]*:/{
        s/^\([^: ][^: ]*\):.*$/Label: \1/
        b
    }' |
    awk '#
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
    function outit(pc, str, code, n, m, s, first){
        first = 1
        while (length(str) > 0) {
            s = substr(str, 1, 4)
            if (first) {
                printf "%04X %s %s\n", pc, s, code
                first = 0
            } else {
                printf "%04X %s\n", pc, s
            }
            pc += length(s) / 2
            str = substr(str, 5)
        }
        return pc
    }
    # main actions
    $1 ~ /^Label:/ {
        label[$2] = pc
        next
    }
    $1 ~ /^.org/ {
        pc = hex2dec($2)
        next
    }
    $1 ~ /^.head/ {
        str = gensub(/"([^"]+)"/,"\\1", "g", $2)
        n = length(str)
        if ((n % 2) == 0)
            str = str " "
        else
            str = str "  "
        printf "\"%s\"\n", str
        if (NF >= 3) {
            if ($3 ~ /immediate/) {
                n = n + 128
            }
        }
        hex = packit(str)
        hex = sprintf("%02x%s", n, hex)
        pc = outit(pc, hex, $0)
        next
    }
    '
done
