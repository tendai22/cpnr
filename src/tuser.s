    .org 0x1000         
dict:
    .dw   dict        ; DICTTOP_ADDR
    .global here_addr
here_addr:
    .dw   entry_end        ; DICTEND_ADDR
    .global last_addr
last_addr:
    .dw   entry_head        ; DICTENTRY_ADDR
    .dw   entry_end        ; DP_ADDR
    .dw   entry_head        ; LAST_ADDR
    .dw   0xe000        ; UP_ADDR
    .dw   0    ; USIZE_ADDR
    .dw   0    ; S0_ADDR
    .dw   0    ; R0_ADDR
    .dw   0    ; TIB_ADDR
    .dw   0        ; STATE_ADDR
    .dw   0        ; DEBUG_ADDR
    .dw   10        ; BASE_ADDR
    .dw   0    ; HALT_ADDR
    .dw   COLON        ; COLON_ADDR
    .dw   SEMI        ; SEMI_ADDR
    .dw   do_dolit        ; LITERAL_ADDR
    .dw   0        ; PAD_ADDR
    .dw   0        ; IN_ADDR
    .dw   1        ; STRICT_ADDR
    .dw   0        ; CSP_ADDR
    .dw   0        ; CROSS_ADDR
    .dw   0    ; COLD_ADDR
    .dw   10        ; #field_addr
    .dw   0        ; #base_addr
    .dw   0        ; OUTER_ADDR
    .dw   0    ; END_ADDR
