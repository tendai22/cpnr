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
    function hex2dec(s){
        if (s !~ /^0x/)
            s = "0x" s
        return strtonum(s) 
    }
    function ascii(c){
        index("!\"#$%'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQR)
    }
    $1 ~ /^Label:/ {
        label[$2] = pc
        next
    }
    $1 ~ /^.org/ {
        pc = hex2dec($2)
        next
    }
    $1 ~ /^.head/ {
        str = $2 " "
        n = length($2)
        if (NF >= 3) {
            if ($3 ~ /immediate/)
                n |= 128
        }
        if ((length(str) % 2) == 0)
            str = str " "
        
    }
    '