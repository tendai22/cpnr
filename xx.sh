#! /bin/sh
awk '
function ascii(s, n){
    n = index(" !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvqxyz{|}~", (s ""))
    print s
    return n > 0 ? n + 31 : n
}
{
    s = gensub(/"([^"]+)"/, "\\1", "g", $0)
    print $0 " -> " s
    next

    n = length($0 "")
    for (i = 1; i <= n ; ++i) {
        x = substr($0 "", i, 1)
        print x, ascii(x)
    }
}'
