\ base.f ... cpnr secondary base word definition

\ memory map
0x1000 constant ROMSTART
0x2000 constant ROMSIZE
0x4000 constant RAMSTART
0x4000 constant RAMSIZE

0x1000 constant DICT_START
0x4000 constant USER_START
0xff00 constant STACK_END

STACK_END constant DSTACK_END
DSTACK_END 0x100 - constant RSTACK_END

\ CELLS
2 constant cells

\ uservar address
USER_START      constant LAST_ADDR
USER_START  2 + constant H_ADDR
USER_START  4 + constant S0_ADDR
USER_START  6 + constant STATE_ADDR
USER_START  8 + constant BASE_ADDR
USER_START 10 + constant HALT_ADDR
USER_START 12 + constant COLON_ADDR
USER_START 14 + constant SEMI_ADDR
USER_START 16 + constant LITERAL_ADDR
USER_START 18 + constant DOCONS_ADDR

\ here/allot/last/immediate
: here H_ADDR @ ;
: allot H_ADDR @ + H_ADDR ! ;
: last LAST_ADDR @ ;
: immediate last c@ 0x80 or last c! ;
: , ( comma) here ! cells allot ;

\
\ control structure
\

\ jump operand mark/resolve
: >mark here ;
: >resolve here swap ! ;
: <mark here ;
: <resolve here ! cells allot ;

\ if-else-then
: if literal jz , >mark cells allot ; immediate
: then >resolve ; immediate
: else literal jmp , >mark cells allot swap >resolve ; immediate

\ do ... loop
: do ( limit index -- )
  literal >r , <mark ; immediate
: i literal r2> dup >r2 ;
: _loop dup ( limit limit)
        r2> 1 + dup >r2 ( limit index)
        > ;
: loop_ rswap drop drop ;
: loop literal _loop , literal jz , <resolve literal loop_ , ; immediate


