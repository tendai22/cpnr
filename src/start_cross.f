variable dA
dup DICTTOP_ADDR 0x41 .ps - dA !
: start_cross \ ( org --- )
   DP_ADDR dup dA - 
   \ keep host dicttop on the stack
   LAST_ADDR @ swap  \ ( cross org )
   \ copy new user to host
   dup dicttop END_ADDR dicttop - 0x42 .ps cmove \ ( from to n --- )
   10 #field_addr !
   CROSS_ADDR 0x43 .ps !   \ write cross to host user area,
                  \ and start cross build
   ;
0x44 .ps cr
\ start_cross
