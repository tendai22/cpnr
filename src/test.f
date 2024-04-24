\ base.f ... cpnr secondary base word definition

\ minimum user variables
: LAST_ADDR     0x4000 ;
: H_ADDR        0x4002 ;
: S0_ADDR       0x4004 ;
: STATE_ADDR    0x4006 ;
: BASE_ADDR     0x4008 ;
: HALT_ADDR     0x400a ;
: COLON_ADDR    0x400c ;
: SEMI_ADDR     0x400e ;
: LITERAL_ADDR  0x4010 ;
: DOCONS_ADDR   0x4012 ;
: DEBUG_ADDR    0x4014 ;

: debug DEBUG_ADDR ! ;
0 debug

: base BASE_ADDR @ ;

\ cells
: cells 2 ;

\
\ dictionary/compilation
\

\ here/allot/last/immediate
: here H_ADDR @ ;
: allot H_ADDR @ + H_ADDR ! ;
: last LAST_ADDR @ ;
: immediate last c@ 0x80 or last c! ;
: , ( comma) here ! cells allot ;

\ link_addr ( addr -- link-addr )
: link_addr
   dup c@      \ addr c
   31 and      \ addr n (= c&0x1f)
   dup 1 and   \ addr n (n&1)
   cells + +   \ addr (n +(n&1)+2)
   + ;

\ code_addr ( addr -- code-addr )
   link_addr cells + ;

\ create
: create
   32 word drop
   last              \ last
   here link_addr    \ last link_pos
   dup rot swap      \ link_pos last link_pos
   !                 \ STAR(link_pos) = last 
   here LAST_ADDR !  \ link_pos
   2 cells * + H_ADDR !
   ;

\
\ does>
\
\ : puship rsp @ ;

: (does)
   last link_addr cells +  \ code_addr
   rsp @            \ get semi addr
   cells +                 \ get colon addr
   swap !            \ STAR(code_addr) = colon_addr
   ;

\ does>
: does>
   compile (does)
   SEMI_ADDR @ ,
   0xc005 ,       \ m_startdoes
   COLON_ADDR @ cells + ,  \ colon bincode
   ; immediate

: constant
   create , does> @ ;
: variable
   create cells allot does> ; 

\ test constant
\ 100 constant foo
\ foo .

\ constants
0xff00 constant STACK_END
STACK_END constant DSTACK_END
DSTACK_END 0x100 - constant RSTACK_END

\ memory map
0x1000 constant ROMSTART
0x2000 constant ROMSIZE
0x4000 constant RAMSTART
0x4000 constant RAMSIZE

0x1000 constant DICT_START
0x4000 constant USER_START

\ uservar address

\ inner interpreter vector



: debug DEBUG_ADDR ! ;
0 debug

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
: r3@   \ index in a word execution
   4 cells * rsp + @ ;
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
   r>
   2 cells * +rsp 
   >r ;

: do
   compile (do)
   <mark ; immediate


: i ( R: index limit ret-addr)
   r1@ ;
: j
   r3@ ;

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
0 debug

\ 2 nested loop
: baka
    3 1 do i . bl emit
            3 1 do i . j . bl emit loop cr
        loop ;

\
\ operators
\
: 2dup over over ;
: < - 0x8000 and ;
: mod /mod drop ;

\
\ accept
\
: _p++ ( c addr -- c addr+1 )
   dup rot  \ addr addr c
   dup rot  \ addr c c addr
   c! swap  \ c addr
   1 +      \ c addr++
   ;

: s0 S0_ADDR @ ;

: dump ( n addr -- )
    1 rot swap    \ addr n 1
    do            \ addr
      dup c@ . space
      1 + loop drop ;

\ fill 
: fill ( addr n c -- )
  rot rot  \ c addr n
  0 do _p++ loop drop drop ;

\
\ pictured output ... numeric formatted
\

\ nbuf
: #nb 32 0xe000 + ;
: #i #nb c@ ;
: #np #nb dup c@ + ;
: #i-- #nb dup c@ 1 - swap c! ;
variable #field_addr
: #field #field_addr @ ;
variable #BASE_ADDR
: #base #BASE_ADDR @ ;

8 #field_addr !

\ <# ... prepare numeric conversion
: <# 
   #field #nb c! 
   #nb 1 + #field 32 fill ;

: !#p ( c -- )
   #nb #i + swap c!
   #i-- ;

: i2a ( n -- c )
   dup 36 > if drop 0 else 
   dup 10 < if 0x30 + else
   10 - 0x41 + then then ;

: # ( u -- n/10 )
   not if
      #base /mod swap ( n -- n/10 n%10 ) 
      i2a #np c!
      #i-- 
   then ;

: xx #field 1 + #nb dump ;
