    .area   CODE    (ABS)
    .org      0x1000
    .trace
dict:
    .dw   dict         ; xx 8000 DICTTOP_HEAD
    .globl here_addr
here_addr:
    .dw   entry_end    ; xx 8002 DICTEND_HEAD
    .globl last_addr
last_addr:
    .dw   entry_head   ; xx 8004 DICTENTRY_HEAD
    .dw   0            ; xx 8006 HALT_HEAD
    .dw   COLON        ; xx 8008 COLON_HEAD
    .dw   SEMI         ; xx 800a SEMI_HEAD
    .dw   do_dolit     ; xx 800c LITERAL_HEAD
    .dw   DODOES       ; xx 800e DODOES_HEAD
    .dw   0            ; xx 8010 m_call
    .dw   0            ; xx 8012 vBDQOUTE
    .dw   0            ; xx 8014 vBRANCH
    .dw   0            ; xx 8016 vDOLIT
    .dw   do_s_dolit   ; xx 8018 vSLIT
    .dw   0            ; xx 801a vPDO
    .dw   0            ; xx 801c vPLOOP
    .dw   0            ; xx 801e vPPOSTLOOP
    .dw   0            ; xx 8020 vQBRANCH
    .dw   0            ; xx 8022 vSDOLIT
    .dw   0            ; xx 8024 vPDOES
    .dw   0            ; xx 8026 COLD_HEAD
    .dw   0            ; xx 8028 EHEAD_HEAD
    .dw   0xe000       ; xx 802a UP_HEAD
    .dw   0            ; xx 802c S0_HEAD
    .dw   0            ; xx 802e R0_HEAD
    .dw   0            ; xx 8030 TIB_HEAD
    .dw   entry_end    ; xx 8032 DP_HEAD
    .dw   entry_head   ; xx 8034 LAST_HEAD
    .dw   0            ; xx 8036 STATE_HEAD
    .dw   0            ; xx 8038 DEBUG_HEAD
    .dw   10           ; xx 803a BASE_HEAD
    .dw   0            ; xx 803c PAD_HEAD
    .dw   0            ; xx 803e IN_HEAD
    .dw   1            ; xx 8040 STRICT_HEAD
    .dw   0            ; xx 8042 CSP_HEAD
    .dw   0            ; xx 8044 CROSS_HEAD
    .dw   10           ; xx 8046 #field_addr
    .dw   0            ; xx 8048 #base_addr
    .dw   0            ; xx 804a OUTER_HEAD
    .dw   0            ; xx 804c vexecute
    .dw   0            ; xx 804e verror
    .dw   0            ; xx 8050 vabort
    .dw   0            ; xx 8052 dpl
    .dw   0            ; xx 8054 warning
    .dw   0            ; xx 8056 dA
    .dw   0            ; xx 8058 dU
    .dw   0            ; xx 805a END_HEAD

COLON:
    m_colon1
    m_jmp     NEXT
SEMI:   
    .dw .+2
    m_semi1
NEXT:   
    m_next1
RUN:    
    m_run
DODOES:
    m_exch
    m_jmp   COLON


entry_000:
e_nop:
    .head   "nop"
    .dw     0
do_nop:
    .dw     .+2
    m_nop
    m_jmp NEXT

entry_001:
e_halt:
    .head   "halt"
    .dw     entry_000
do_halt:
    .dw     .+2
    m_halt
    m_jmp NEXT

entry_002:
e_execute:
    .head   "execute"
    .dw     entry_001
do_execute:
    .dw     .+2
    m_execute
    m_jmp NEXT

entry_003:
e_trap:
    .head   "trap"
    .dw     entry_002
do_trap:
    .dw     .+2
    m_trap
    m_jmp NEXT

entry_004:
e_quote:
    .head   "'"
    .dw     entry_003
do_quote:
    .dw     .+2
    m_quote
    m_jmp NEXT

entry_005:
e_exch:
    .head   "exch"
    .dw     entry_004
do_exch:
    .dw     .+2
    m_exch
    m_jmp NEXT

entry_006:
e_call:
    .head   "call"
    .dw     entry_005
do_call:
    .dw     .+2
    m_call
    m_jmp NEXT

entry_007:
e_dolit:
    .head   "dolit"
    .dw     entry_006
do_dolit:
    .dw     .+2
    m_dolit
    m_jmp NEXT

entry_008:
e_branch:
    .head   "branch"
    .dw     entry_007
do_branch:
    .dw     .+2
    m_branch
    m_jmp NEXT

entry_009:
e_qbranch:
    .head   "?branch"
    .dw     entry_008
do_qbranch:
    .dw     .+2
    m_qbranch
    m_jmp NEXT

entry_010:
e_s_dolit:
    .head   "s_dolit"
    .dw     entry_009
do_s_dolit:
    .dw     .+2
    m_s_dolit
    m_jmp NEXT

entry_011:
e_cdepo:
    .head   "c!"
    .dw     entry_010
do_cdepo:
    .dw     .+2
    m_cdepo
    m_jmp NEXT

entry_012:
e_depo:
    .head   "!"
    .dw     entry_011
do_depo:
    .dw     .+2
    m_depo
    m_jmp NEXT

entry_013:
e_cfetch:
    .head   "c@"
    .dw     entry_012
do_cfetch:
    .dw     .+2
    m_cfetch
    m_jmp NEXT

entry_014:
e_fetch:
    .head   "@"
    .dw     entry_013
do_fetch:
    .dw     .+2
    m_fetch
    m_jmp NEXT

entry_015:
e_rot:
    .head   "rot"
    .dw     entry_014
do_rot:
    .dw     .+2
    m_rot
    m_jmp NEXT

entry_016:
e_swap:
    .head   "swap"
    .dw     entry_015
do_swap:
    .dw     .+2
    m_swap
    m_jmp NEXT

entry_017:
e_drop:
    .head   "drop"
    .dw     entry_016
do_drop:
    .dw     .+2
    m_drop
    m_jmp NEXT

entry_018:
e_over:
    .head   "over"
    .dw     entry_017
do_over:
    .dw     .+2
    m_over
    m_jmp NEXT

entry_019:
e_dup:
    .head   "dup"
    .dw     entry_018
do_dup:
    .dw     .+2
    m_dup
    m_jmp NEXT

entry_020:
e_rsp:
    .head   "rsp"
    .dw     entry_019
do_rsp:
    .dw     .+2
    m_rsp
    m_jmp NEXT

entry_021:
e_r2s:
    .head   "r>"
    .dw     entry_020
do_r2s:
    .dw     .+2
    m_r2s
    m_jmp NEXT

entry_022:
e_s2r:
    .head   ">r"
    .dw     entry_021
do_s2r:
    .dw     .+2
    m_s2r
    m_jmp NEXT

entry_023:
e_not:
    .head   "not"
    .dw     entry_022
do_not:
    .dw     .+2
    m_not
    m_jmp NEXT

entry_024:
e_xor:
    .head   "xor"
    .dw     entry_023
do_xor:
    .dw     .+2
    m_xor
    m_jmp NEXT

entry_025:
e_or:
    .head   "or"
    .dw     entry_024
do_or:
    .dw     .+2
    m_or
    m_jmp NEXT

entry_026:
e_and:
    .head   "and"
    .dw     entry_025
do_and:
    .dw     .+2
    m_and
    m_jmp NEXT

entry_027:
e_div:
    .head   "/"
    .dw     entry_026
do_div:
    .dw     .+2
    m_div
    m_jmp NEXT

entry_028:
e_mul:
    .head   "*"
    .dw     entry_027
do_mul:
    .dw     .+2
    m_mul
    m_jmp NEXT

entry_029:
e_sub:
    .head   "-"
    .dw     entry_028
do_sub:
    .dw     .+2
    m_sub
    m_jmp NEXT

entry_030:
e_add:
    .head   "+"
    .dw     entry_029
do_add:
    .dw     .+2
    m_add
    m_jmp NEXT

entry_031:
e_gt:
    .head   ">"
    .dw     entry_030
do_gt:
    .dw     .+2
    m_gt
    m_jmp NEXT

entry_032:
e_slashmod:
    .head   "/mod"
    .dw     entry_031
do_slashmod:
    .dw     .+2
    m_slashmod
    m_jmp NEXT

entry_033:
e_emit:
    .head   "emit"
    .dw     entry_032
do_emit:
    .dw     .+2
    m_emit
    m_jmp NEXT

entry_034:
e_key:
    .head   "key"
    .dw     entry_033
do_key:
    .dw     .+2
    m_key
    m_jmp NEXT

entry_035:
e_kbhit:
    .head   "kbhit"
    .dw     entry_034
do_kbhit:
    .dw     .+2
    m_kbhit
    m_jmp NEXT

entry_036:
e_colondef:
    .head   ":"
    .dw     entry_035
do_colondef:
    .dw     .+2
    m_colondef
    m_jmp NEXT

entry_037:
e_semidef:
    .head   ";" 128
    .dw     entry_036
do_semidef:
    .dw     .+2
    m_semidef
    m_jmp NEXT

entry_038:
e_semi2def:
    .head   ";;" 128
    .dw     entry_037
do_semi2def:
    .dw     .+2
    m_semi2def
    m_jmp NEXT

entry_039:
e_dd:
    .head   "dd"
    .dw     entry_038
do_dd:
    .dw     .+2
    m_dd
    m_jmp NEXT

entry_040:
e_lnum:
    .head   "lnum"
    .dw     entry_039
do_lnum:
    .dw     .+2
    m_lnum
    m_jmp NEXT

entry_041:
e_add_rsp:
    .head   "+rsp"
    .dw     entry_040
do_add_rsp:
    .dw     .+2
    m_add_rsp
    m_jmp NEXT

entry_042:
e_getline:
    .head   "getline"
    .dw     entry_041
do_getline:
    .dw     .+2
    m_getline
    m_jmp NEXT

entry_043:
e_outer:
    .head   "outer"
    .dw     entry_042
do_outer:
    .dw     .+2
    m_outer
    m_jmp NEXT

entry_044:
e_sp_at:
    .head   "sp@"
    .dw     entry_043
do_sp_at:
    .dw     .+2
    m_sp_at
    m_jmp NEXT

entry_045:
e_rsp_reset:
    .head   "rp!"
    .dw     entry_044
do_rsp_reset:
    .dw     .+2
    m_rsp_reset
    m_jmp NEXT

entry_046:
e_sp_reset:
    .head   "sp!"
    .dw     entry_045
do_sp_reset:
    .dw     .+2
    m_sp_reset
    m_jmp NEXT

entry_047:
e_dadd:
    .head   "d+"
    .dw     entry_046
do_dadd:
    .dw     .+2
    m_dadd
    m_jmp NEXT

entry_048:
e_madd:
    .head   "m+"
    .dw     entry_047
do_madd:
    .dw     .+2
    m_madd
    m_jmp NEXT

entry_049:
e_mmuldiv:
    .head   "m*/"
    .dw     entry_048
do_mmuldiv:
    .dw     .+2
    m_mmuldiv
    m_jmp NEXT

entry_050:
e_dlt:
    .head   "d<"
    .dw     entry_049
do_dlt:
    .dw     .+2
    m_dlt
    m_jmp NEXT

entry_051:
e_umul:
    .head   "u*"
    .dw     entry_050
do_umul:
    .dw     .+2
    m_umul
    m_jmp NEXT

entry_052:
e_d_dolit:
    .head   "d_dolit"
    .dw     entry_051
do_d_dolit:
    .dw     .+2
    m_d_dolit
    m_jmp NEXT

entry_053:
e_dictdump:
    .head   "dictdump"
    .dw     entry_052
do_dictdump:
    .dw     .+2
    m_dictdump
    m_jmp NEXT

entry_054:
e_bye:
    .head   "bye"
    .dw     entry_053
do_bye:
    .dw     .+2
    m_bye
    m_jmp NEXT

entry_055:
e_ccompile:
    .head   "[compile]" 128
    .dw     entry_054
do_ccompile:
    .dw     .+2
    m_ccompile
    m_jmp NEXT

entry_056:
e_dump:
    .head   "dump"
    .dw     entry_055
do_dump:
    .dw     .+2
    m_dump
    m_jmp NEXT

entry_057:
e_dotps:
    .head   ".ps"
    .dw     entry_056
do_dotps:
    .dw     .+2
    m_dotps
    m_jmp NEXT

entry_end:
    .equ entry_head, entry_057
    .end

