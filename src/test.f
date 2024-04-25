\ base.f ... cpnr secondary base word definition

\ minimum user variables
: LAST_ADDR     0x4000 ;
: H_ADDR        0x4002 ;
: S0_ADDR       0x4004 ;
: STATE_ADDR    0x4006 ;
: BASE_ADDR     0x4008 ;
\ for compile instruction/constant
: COLON_ADDR    0x400c ;
: SEMI_ADDR     0x400e ;
: DEBUG_ADDR    0x4012 ;

: debug DEBUG_ADDR ! ;

