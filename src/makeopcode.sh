#! /bin/sh
cat "$@" |
sed '1i\
BEGIN {
    $a\
}\

    /opcode_base *=/{
        s/^.*\(opcode_base *= *\)\([^;]*\);.*/\2/
        h
        s/^\(.*\)$/    opcode_base = strtonum(\1)/
        b
    }
    /case [^:][^:]*:  *\/\/  *m_/{
        s/case  *\([^ :][^:]*\):  *\/\/  *\(m_[a-zA-Z_][a-zA-Z0-9_*]*\).*$/opcode["\2"] = \1 + opcode_base\
    ropcode[\1 + opcode_base] = "\2"/
        b
    }
    d
'