\
\ cold.f ... cold start routines
\
: init_dump dicttop @ dicttop END_ADDR dicttop - cmove ;
: cold init_dump abort ;
' cold COLD_ADDR !