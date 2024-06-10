: DICTTOP_ADDR     0xe000 ;
: DICTEND_ADDR     0xe002 ;
: DICTENTRY_ADDR   0xe004 ;
: DP_ADDR          0xe006 ;
: LAST_ADDR        0xe008 ;
: UP_ADDR          0xe00a ;
: USIZE_ADDR       0xe00c ;
: S0_ADDR          0xe00e ;
: R0_ADDR          0xe010 ;
: TIB_ADDR         0xe012 ;
: STATE_ADDR       0xe014 ;
: DEBUG_ADDR       0xe016 ;
: BASE_ADDR        0xe018 ;
: HALT_ADDR        0xe01a ;
: COLON_ADDR       0xe01c ;
: SEMI_ADDR        0xe01e ;
: LITERAL_ADDR     0xe020 ;
: PAD_ADDR         0xe022 ;
: IN_ADDR          0xe024 ;
: STRICT_ADDR      0xe026 ;
: CSP_ADDR         0xe028 ;
: CROSS_ADDR       0xe02a ;
: COLD_ADDR        0xe02c ;
: #field_addr      0xe02e ;
: #base_addr       0xe030 ;
: OUTER_ADDR       0xe032 ;
: END_ADDR         0xe034 ;
 DICTTOP_ADDR 256 +   S0_ADDR          !
 DICTTOP_ADDR 512 +   R0_ADDR          !
 DICTTOP_ADDR 256 +   TIB_ADDR         !
 ' halt     HALT_ADDR        !

