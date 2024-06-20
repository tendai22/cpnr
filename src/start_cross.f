\ ( dict_org user_org --- )
0x46 .ps 
USER_ORG_CONST 0x50 .ps - dU 0x51 .ps !
ORG_CONST 0x40 .ps - dA 0x41 .ps !
dA @ h4. space dU @ h4. cr
: cross CROSS_ADDR @ ;
\ 4 debug
LAST_ADDR @ CROSS_ADDR 0x31 .ps !
ORG_CONST dA @ 0x5a .ps + 12 dump
USER_ORG_CONST 12 dump
\ t_DICTEND_HEAD -> DP_ADDR 
\ t_DICTENTRY_HEAD -> LAST_ADDR 
DICTEND_HEAD dA @ + ( dictend_addr ) @ DP_ADDR 0x32 .ps !
DICTENTRY_HEAD dA @ + ( dictentry_addr ) @ LAST_ADDR 0x33 .ps !
\ t_COLON_HEAD -> h_COLON_HEAD
\ t_SEMI_HEAD -> h_SEMI_HEAD
\ t_LITERAL_HEAD -> h_LITERAL_HEAD
COLON_HEAD   dA @ + @ COLON_HEAD 0x44 .ps !
SEMI_HEAD    dA @ + @ SEMI_HEAD 0x45 .ps !
LITERAL_HEAD dA @ + @ LITERAL_HEAD 0x46 .ps !
DODOES_HEAD  dA @ + @ DODOES_HEAD 0x4f .ps !
\ t_HALT_HEAD -> h_HALT_HEAD
' halt                HALT_HEAD 0x47 .ps !
\ t_DICTTOP_HEAD -> h_DICTTOP_HEAD
DICTTOP_HEAD dA @ + @ DICTTOP_HEAD 0x47 .ps !
DICTEND_HEAD dA @ + @ DICTEND_HEAD 0x48 .ps !
DICTENTRY_HEAD dA @ + @ DICTENTRY_HEAD 0x49 .ps !
\ h_S0_HEAD -> t_S0_HEAD
UP_HEAD @ dU @ +   UP_HEAD dA @ + 0x4a .ps !
S0_HEAD @ dU @ +   S0_HEAD dA @ + 0x4b .ps !
R0_HEAD @ dU @ +   R0_HEAD dA @ + 0x4c .ps !
TIB_HEAD @ dU @ +  TIB_HEAD dA @ + 0x4d .ps !
HALT_HEAD @        HALT_HEAD dA @ + 0x4e .ps !

ORG_CONST    dA @ + @ 24 dump
ORG_CONST             24 dump
USER_ORG_CONST 12 dump

