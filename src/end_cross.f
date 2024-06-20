\ copy vectors to target dict
COLON_HEAD dup dA @ + \ src-top dest-top
EHEAD_HEAD COLON_HEAD - \ num
cmove
\ 1 0x1036 ! 
0x1000 36 dump

