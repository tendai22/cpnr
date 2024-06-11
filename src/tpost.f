DP_ADDR dup     dA @ + swap @ swap 0x43 .ps ! \ ( cross org )
LAST_ADDR dup   dA @ + swap @ swap 0x43 .ps ! \ ( cross org )
DICTEND_ADDR    dA @ + DP_ADDR @ swap !
DICTENTRY_ADDR  dA @ + LAST_ADDR @ swap !
