\
\ cold.f ... cold start routines
\
\ cmove ( from to count --- )
: init_dump DP_HEAD 0x58 .ps DP_ADDR 0x58 .ps END_ADDR 0x58 .ps DP_ADDR - 0x58 .ps cmove ;
: cold 0x57 .ps ' init_dump dd init_dump 3 debug abort ;
' cold COLD_HEAD !