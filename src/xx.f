variable dA
DICTTOP_ADDR 0x41 .ps - dA !
\ set cross
LAST_ADDR @ CROSS_ADDR !
CROSS_ADDR dup h4. space @ h4. cr
\ copy addrs
DP_ADDR dup dA @ + 0x42 .ps @ swap !
LAST_ADDR dup dA @ + @ swap !
COLON_ADDR dup dA @ + @ swap !
SEMI_ADDR dup dA @ + @ swap !
LITERAL_ADDR dup dA @ + @ swap !
DICTTOP_ADDR 48 dump
