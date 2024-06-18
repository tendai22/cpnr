\
\ cold.f ... cold start routines
\
\ cmove ( from to count --- )
: init_dump DP_HEAD dup @ END_ADDR DP_ADDR - ( 0x41 .ps ) cmove ;
: cold init_dump abort ;
\ ' cold COLD_HEAD !