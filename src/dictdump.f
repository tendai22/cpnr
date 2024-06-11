\
\ dictdump.f ... store user area to top of dict
\   and dictdump to "forth.bin"
\
dicttop dicttop @ END_ADDR dicttop - cmove
dicttop @ here last dictdump
