#! /bin/sh
cat "$@" |
sed -n '/^  *end$/q
/^[0-9A-Fa-f][0-9A-Fa-f]* [0-9A-Za-z][0-9A-Za-z]*/{
    s/^\([0-9A-Za-z][0-9A-Za-z][0-9A-Za-z][0-9A-Za-z]\) \([0-9A-Za-z][0-9A-Za-z][0-9A-Za-z][0-9A-Za-z]\).*$/=\1 \2/
    s/^\([0-9A-Za-z][0-9A-Za-z][0-9A-Za-z][0-9A-Za-z]\) \([0-9A-Za-z][0-9A-Za-z]\).*$/=\1 \2/
    p
    b
}
'