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
USER_START 20 + constant DEBUG_ADDR

\ here/allot/last/immediate
: here H_ADDR @ ;
: allot H_ADDR @ + H_ADDR ! ;
: last LAST_ADDR @ ;
: immediate last c@ 0x80 or last c! ;
: , ( comma) here ! cells allot ;

: debug DEBUG_ADDR ! ;
1 debug

\
\ control structure
\

\ jump operand mark/resolve
: >mark here cells allot ;
: >resolve here swap ! ;
: <mark here ;
: <resolve here ! cells allot ;

\ if-else-then
: if compile ?branch >mark ; immediate
: then >resolve ; immediate
: else compile branch >mark swap >resolve ; immediate

\ do ... loop

\ loop-structure: do ... loop 実行構造をリターンスタックに置く
\ (limit index -- ) .. doでデータスタックから2要素をリターン
\ スタックに移動させる

\ リターンスタック要素操作用

: r1@
   2 cells * rsp + @ ;
: r2@   \ index in a word execution
   3 cells * rsp + @ ;
: r1!   \ store a word in index
   2 cells * rsp + ! ;
\ : +rsp \ add a word to rsp, defined as opcode

\ limit, index をリターンスタックに移動させる、
\ >rを使うので逆順になる
\ do 呼び出し時のリターンアドレスがすでに乗っている。
\ いったんデータスタックに移し
\ 3ワードリターンスタックに戻す。
: (do)  \ ( limit index -- ) ( R: -- index limit )
   swap
   r>           \ index limit ret-address
   rot rot      \ ret-address index limit
   >r >r >r ;   \ --> limit index ret-address
: (post-loop) \ restore return stack 
   3 cells * +rsp ;

: do
   compile (do)
   <mark ; immediate


: i ( R: index limit ret-addr)
   r1@ ;

: (loop)         \ ( delta -- )
    r1@ +        \ new index
    dup r1!  \ save new index, new index remains
    r2@         \ index limit
    >       \ (limit -1) if i+d > limit, (limit 0) if i+d <= limit 
    \ falling down to ?branch
    ;

: loop  \ limit -- limit if loop remains | none if loop exits)
    compile literal
    1 here ! cells allot   \ compile 1 as delta)
                        \ limit limit iaddr index 1 
    compile (loop)      \ limit -1|0)
    compile ?branch
    <resolve
    compile (post-loop)
    ; immediate


\ begin ... until
: begin <mark ; immediate
: until ( flag -- )
   compile ?branch <resolve 
   ; immediate

 \ begin ... f while ... repeat
 : while ( f -- )
   compile ?branch >mark ; immediate
: repeat ( -- )
   compile branch
   swap <resolve 
   >resolve ; immediate

\ test words
: test1 3 begin dup . cr 1 - dup not until drop ;
: test2 3 begin dup while dup . cr 1 - repeat drop ;

: aho 3 1 do i . cr loop ;
2 debug


