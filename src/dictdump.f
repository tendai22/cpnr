\
\ dictdump.f ... store user area to top of dict
\   and dictdump to "forth.bin"
\
\ revert RAM user vars to dicttop
DP_ADDR @ DP_HEAD !
DP_ADDR @ DICTEND_HEAD !
LAST_ADDR @ LAST_HEAD !
LAST_ADDR @ DICTENTRY_HEAD !

dicttop @ here last dictdump
bye
