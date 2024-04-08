#! /bin/sh
cat "$@" |
sed '
    /opcode_base *=/{
        s/^.*\(opcode_base *= *\)\([^;]*\);.*/\2/
        h
        s/^\(.*\)$/    opcode_base = \1/
        b
    }
    /case [^:][^:]*:  *\/\/  *m_/{
        s/case  *\([^ :][^:]*\):  *\/\/  *\(m_[a-zA-Z_][a-zA-Z0-9_*]*\).*$/opcode["\2"] = \1 + /
        G
        s/\n/ /g
        b
    }
    d
'