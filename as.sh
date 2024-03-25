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
    $1 ~ /^Label:/ {
        label[$2] = pc
    }
    $1 ~ /^.org/ {
        pc = $2
    }
    $1 ~ /^.
    '