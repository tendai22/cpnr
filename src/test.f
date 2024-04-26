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

\ debug
: debug DEBUG_ADDR ! ;
1 debug
\ base
: base BASE_ADDR @ ;
10 BASE_ADDR !

\ signbit
: signbit 0x8000 ;

\ cells
: cells 2 ;

\
\ simple ones
\
: 1+ 1 + ;

\
\ dictionary/compilation
\