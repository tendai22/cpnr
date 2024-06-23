\
\ dictdump.f ... store user area to top of dict
\   and dictdump to "forth.bin"
\
\ revert RAM user vars to dicttop
DP_ADDR DP_HEAD END_ADDR DP_ADDR - 0x5a .ps cmove

DP_ADDR @ DP_HEAD dA @ + 0x61 .ps !
DP_ADDR @ DICTEND_HEAD dA @ + 0x62 .ps !
LAST_ADDR @ LAST_HEAD dA @ + 0x63 .ps !
LAST_ADDR @ DICTENTRY_HEAD dA @ + 0x64 .ps !

0x8000 48 dump
0xf000 24 dump
dicttop @ here last dictdump
bye
