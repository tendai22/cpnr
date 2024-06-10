0x1000
0x46 .ps 
variable dA
dup DICTTOP_ADDR 0x40 .ps - dA 0x41 .ps !
dA @ h4. cr
: cross CROSS_ADDR @ ;
\ 4 debug
LAST_ADDR @ CROSS_ADDR 0x31 .ps ! 
DP_ADDR dup dA @ 0x32 .ps + 0x33 .ps @ 0x34 .ps swap 0x42 .ps !
LAST_ADDR dup dA @ + @ swap 0x43 .ps ! \ ( cross org )
10 #field_addr !
drop
0x44 .ps cr
