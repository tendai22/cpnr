\ base.f ... cpnr secondary base word definition

\ minimum user variables
: LAST_ADDR     0x4000 ;
: H_ADDR        0x4002 ;
: S0_ADDR       0x4004 ;
: STATE_ADDR    0x4006 ;
: BASE_ADDR     0x4008 ;
\ for compile instruction/constant
: COLON_ADDR    0x400c ;
: SEMI_ADDR     0x400e ;
: DEBUG_ADDR    0x4012 ;

\ debug
: debug DEBUG_ADDR ! ;
0 debug

\ base
: base BASE_ADDR @ ;
10 BASE_ADDR !

\ signbit
: signbit 0x8000 ;

\ cells
: cells 2 ;

\
\ simple ones
\
: 1+ 1 + ;

\
\ dictionary/compilation
\

\ here/allot/last/immediate
: here H_ADDR @ ;
: allot H_ADDR @ + H_ADDR ! ;
: last LAST_ADDR @ ;
: immediate last c@ 0x80 or last c! ;
: , ( comma) here ! cells allot ;
: ] ( -- ) 1 STATE_ADDR ! ; immediate
: [ ( -- ) 0 STATE_ADDR ! ; immediate

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

\
\ end of 1st stage definition
\

: hex 16 BASE_ADDR ! ;
: decimal 10 BASE_ADDR ! ;

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

\ primitives
: bl 32 ;
: cr 13 emit 10 emit ;
: h H_ADDR ;

\
\ control structure
\

\ jump operand mark/resolve
: >mark here cells allot ;
: >resolve here swap ! ;
: <mark here ;
: <resolve here ! cells allot ;

\ ======================================
\ if-else-then
\
: if compile ?branch >mark ; immediate
: then >resolve ; immediate
: else compile branch >mark swap >resolve ; immediate

\ ======================================
\ operators
\
\ we can use '<'
: boolean ( n -- ffff|0000 ) 
   if -1 else 0 then ;
: negate ( n -- 0000|ffff )
   0 swap -  ;
: 2dup over over ;
: = - not ;
: != - not not ;
: < swap > ;
: 0= not ;
: <= - dup 0 < swap 0= or ;
: >= swap <= ;
: false 0 ;
: true -1 ;

\ ======================================
\ do ... loop
\

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
   dup 0 > if
      r1@ +        \ new index
      dup r1!  \ save new index, new index remains
      r2@         \ index limit
      >       \ (limit -1) if i+d > limit, (limit 0) if i+d <= limit 
   else
      r1@ +        \ new index
      dup r1!  \ save new index, new index remains
      r2@         \ index limit
      <       \ (limit -1) if i+d > limit, (limit 0) if i+d <= limit 
   then
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

: +loop  \ 
    compile (loop)      \ limit -1|0)
    compile ?branch
    <resolve
    compile (post-loop)
    ; immediate

: leave
   r2@ r1! ;

: unloop
   cells 2 1 + * +rsp ;
   \ 2 for limit, index
   \ 1 for return address of unloop itself

\
\ begin ... until
\
: begin <mark ; immediate
: until ( flag -- )
   compile ?branch <resolve 
   ; immediate

\
\ begin ... f while ... repeat
\
: while ( f -- )
   compile ?branch >mark ; immediate
: repeat ( -- )
   compile branch
   swap <resolve 
   >resolve ; immediate

\ test words
\ : test1 3 begin dup . cr 1 - dup not until drop ;
\ : test2 3 begin dup while dup . cr 1 - repeat drop ;
\ : aho 3 1 do i . cr loop ;
\ 2 nested loop
\ : baka
\    3 1 do i . space
\            3 1 do i . j . space loop cr
\        loop ;
\ unloop test
\ : aho 5 1 do i . cr i 3 = if unloop exit then loop ;

\ ==== end of primary defintions

: s0 S0_ADDR @ ;

\ stack operations
\ opcode rot
\ opcode swap
\ opcode drop
\ opcode over
\ opcode dup 
: ?dup ( x -- 0|x x)
   dup if dup then ;
: depth ( -- +n )
   \ return the number of cells on the stack
   sp s0 swap - 2 / ;
: nip ( x1 x2 -- x2 )
   swap drop ;
: pick ( +n -- x )
   \ place a copy of the nth stack entry, 0th is tos
   sp swap 1+ cells * + @ ;


: min 2dup - signbit and not if swap then drop ;
: max 2dup - signbit and if swap then drop ;
: mod /mod drop ;

: _p++ ( c addr -- c addr+1 )
   dup rot  \ addr addr c
   dup rot  \ addr c c addr
   c! swap  \ c addr
   1 +      \ c addr++
   ;

\ fill 
: fill ( addr n c -- )
  rot rot  \ c addr n
  0 do _p++ loop drop drop ;

\ ================================================
\ pictured output ... numeric formatted
\

\ number conversion buffer
\ size 32 bytes, right adjustment 
\ (tail of 32bytes are occupied)
\ #nb[0]: index
\ #nb[1-31]: character buffer
: #nb 32 0xe000 + ;
: #i #nb c@ ;
: #np #nb dup c@ + ;
: #i-- #nb dup c@ 1 - swap c! ;
variable #field_addr
: #field #field_addr @ ;
variable #base_addr
: #base #base_addr @ ;

10 #base_addr !
8  #field_addr !

\ <# ... prepare numeric conversion
: <#
   base #base_addr !
   #field #nb c! 
   #nb 1 + #field bl fill ;

: !#p ( c -- )
   #nb #i + swap c!
   #i-- ;

: i2a ( n -- c )
   dup 36 > if drop 0 else 
   dup 10 < if 0x30 + else
   10 - 0x41 + then then ;

: # ( u -- n/10 )
   #base /mod swap ( n -- n/10 n%10 ) 
   i2a #np c!
   #i-- ;

: #s ( u -- )
   begin # dup not until ;

: #> ( -- ) \ return string for `TYPE` 
   drop #nb #i + #field #i - ;

: hold  ( c -- ) \ append a char to nbuf
   #np c! #i-- ;

: type ( addr u -- ) \ print a string
   1 do dup i + c@ emit loop drop ;

: sign ( n xx - n xx ) \ print '-' if n is minus
   swap dup signbit and if 45 #np c! #i-- then swap ;

: abs dup signbit and if 0 swap - then ;

: . dup abs <# #s sign #> type drop ;

: #hex 16 #base_addr ! ;
: #decimal 10 #base_addr ! ;
: #dec 10 #base_addr ! ;

: h2.  \ print hex number
  <# #hex # # #> type ;

: h4.  \ print hex number
  <# #hex # # # # #> type ;

: dump ( addr n -- ) \ simple dump
   swap dup h4. space
   swap
   256 min
   1             \ addr n 1
   do            \ addr
      dup c@ h2. space
      i 16 mod not if cr then
      1+ loop drop ;

\ : xx #field 1 + #nb dump ;

\ space ( -- ) .. type a space
: space bl emit ;

\ spaces ( n -- ) .. n spaces
: spaces 1 do bl emit loop ;

\ char ( -- c ) \ put an ascii value
: char bl word 1+ c@ ; immediate

\ exit
: exit compile semi ; immediate

\ outer interpreter

\ ここまでは、acceptは機械語命令m_acceptで実現されていた。
\ ターゲット上で動作するacceptは、キー入力をバッファに集めてゆく
\ だけのもので、メモリ上テキスト、ディスクバッファ上テキストは
\ 対象外とする。
\ 行編集はサポートしない。行編集は別途用意することとする。

\ ===============================================
\ accept
\
\ input stream, integrated keyin and 
\ disk/memory source reader
\
: pad 0xd000 ;
: in_p ( -- addr )
   pad dup c@ + 1+ ;
: inc_p ( n -- )
   pad dup c@ 1+ swap c! ;
: in_rest ( -- n )
   127 pad c@ - ;
variable >in
variable outer_flag
1 outer_flag !

\ input buffer, s0, 128bytes
\ top(*s0) holds the current index
\ accept ... primitive ( -- ), check s0 c@
: accept ( -- )
   0 pad c!              \ i = 1
   127 1 do
      getch 
      dup 10 = if
         drop leave
      else
      dup 0 >= if       \ valid char
         in_p c!        \ *p = c
         inc_p          \ i++ 
      then then 
   loop 
   1 >in ! ;

: w_getch 
   pad >in @ + c@ \ dup h2. space    \ pad[i]
;
: w_i++ 
   >in dup @ 1+ swap ! ;

\ ======================================
\ .stack ... debug word
\
: .stack 
   literal [ char [ , ] emit
   sp s0 != if
      sp s0 cells - do
         i @ h4. space 
      0 cells - +loop
   then 
   literal [ char ] , ] emit
   ;

\ ====================================
\ word ... extract a word
\
: w_rest pad c@ >in @ - ;

: word ( delim -- addr )
   w_rest 0 <= if 0 here c! drop 0 exit then
   begin w_rest 0 > over w_getch = and while
      w_i++
      \ >in @ h4. space
   repeat
   \ accumulate a word
   1     \ destination index h[i]
   begin w_rest 0 >= 2 pick w_getch != and while
      here over + w_getch swap c!  \ here[i] = pad[i]
      \ w_getch emit space
      1+ w_i++
   repeat
   dup here + 2 pick swap c!
   1 - here c!          \ put count on here
   drop
   here \ 10 dump
   ;

\ accept-word test
\ : wtest accept begin 32 word dup while 10 dump cr repeat drop ;

