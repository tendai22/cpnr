\
\ dictdump.f ... store user area to top of dict
\   and dictdump to "forth.bin"
\
dicttop dicttop @ END_ADDR dicttop - cmove ( from to count --- )
dicttop @ here last dictdump
bye
