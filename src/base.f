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
: code_addr 
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
: pad S0_ADDR @ ;
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
: =whitespace ( char -- flag )
   dup  13 = 
   over 10 = or
   over 8 = or
   swap drop ;

: accept ( -- )
   0 pad c!              \ i = 1
   127 1 do
      getch 
      dup 0 = if
         leave
      else dup =whitespace if
         leave
      else dup 0 = if
         leave
      else dup 0 > if       \ valid char
         in_p c!        \ *p = c
         inc_p          \ i++ 
      then then then then
   loop
   bl in_p c!
   inc_p 
   1 >in ! 
   drop ;

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

\
\ words for debugging
\
: .ps ( char -- ) \ dump stack with char
   emit .stack cr ;
: .hd             \ dump here buffer
   here 10 dump cr ;

\ ====================================
\ word ... extract a word
\
: h++ here c@ 1+ here c! ;
: hptr here dup c@ + 1+ ;
: word ( delim -- addr )
   0              \ ( -- delim flag ) ... flag show if it has started to store chars
   0 here c!      \ clear here[0]
   pad dup dup c@ + swap ( 0x30 .ps ) \  limit-index
   >in @ +        \ start-index
   do dup 0 = if  \ skip it
         i c@ 2 pick ( 0x41 .ps ) != if 1+ then  \ flag++
      then
      dup 1 = if  \ store it
         i c@ 2 pick ( 0x42 .ps ) = i c@ 0 = or if 1+ else   \ flag++
            i c@                    \ src char
            hptr ( 0x43 .ps ) c!     \ store it
            h++      \ increment counter
            ( .hd )
         then
      then
      dup 2 = if  \ put trailing delim
         over
         hptr ( 0x44 .ps ) c!
         \ do not increment h++
         1+       \ increment flag
         leave
      then
      >in @ 1+ >in ( 0x45 .ps ) ! \ increment >in  
   loop
   drop drop here 
   ( .hd ) ;

\ test word for accept-word
\ : aho accept begin bl word c@ while .hd repeat ;

\ =========================
\ find ... search a word in the dictionary
: compare ( c-addr1 u1 c-addr2 u2 -- n )
   \ 3 pick 2 dump 1 pick 2 dump cr
   0     \ dummy for first drop
   3 pick 2 pick min 1 -    \ max index (len - 1)
   0 do     \ c-addr1 u1 c-addr2 u2
      drop
      3 pick i + c@  \ c-addr1 u1 c-addr2 u2 i c1
      2 pick i + c@  \ c-addr1 u1 c-addr2 u2 i c1 c2
      \ 0x32 emit .stack cr
      -           \ c-addr1 u1 c-addr2 u2 i (c1-c2)
      \ 0x33 emit .stack cr
      dup 0 != if leave then 
   loop
   \ now the result is 0|-1|1,
   \ if 0, compare u1 and u2
   dup 0 = if 3 pick 2 pick - swap drop then
   \ ok got it, erase 4 args
   swap drop swap drop swap drop swap drop 
;

\ for test command
: align ( addr -- addr2 )
   cells + 1 - cells / cells * ;

0xe000 constant tmp 
: tcom
   here  \ save it on stack
   tmp H_ADDR !
   accept
   32 word
   1+
   \ 0x30 emit .stack cr
   here dup c@ 2 + + align H_ADDR ! \ new address
   32 word
   1+
   \ 0x31 emit .stack cr
   rot H_ADDR !
   1 pick 1 - c@ swap dup 1 - c@
   tmp 16 dump cr
   .stack cr
   compare
   ;

\ link_addr ( entry -- link )
: link_addr
   dup c@ 0x1f and   \ n = *entry & 0x1f
   dup 1 and + 2 + +
;

\ prev_link ( entry -- prev-entry )
: prev_link
   link_addr @
;

\ print name
: entry_name ( entry -- )
   c@ 0x1f and \ length
   1 do dup i + c@ emit loop space ;

\ test
: ltest
   here h4. bl emit cr
   last
   begin dup while
      dup h4. bl emit dup entry_name cr
      prev_link repeat drop ;

\ find   ( c-addr -- c-addr 0 | xt 1 | xt -1 )
\    returns 0 if not found, c-addr remains,
\            1 if an immediate word found
\           -1 if non-immediate word found
: -find
   DEBUG_ADDR @
   0 debug
   0 swap      \ c-addr 0 last
   begin dup while   \ repeat until link is null
      \ c-addr 0 link
      dup
      \ c-addr 0 link link
      dup c@ 0x1f and swap 1+ swap    \ entry string length
      \ c-addr 0 link link+1 len
      4 pick dup c@ swap 1+ swap
      \ c-addr 0 link link+1 len c-addr+1 len
      \ 0x4C emit .stack 
      compare
      \ 0x43 emit .stack cr
      \ c-addr 0 link 0|-1|1 
      0 != if prev_link else   \ c-addr 0 prev_link
      \ make return code
      \ c-addr 0 link -> c-addr link 0
      swap then
      \ 0x55 emit .stack cr 
   repeat
   \ here, c-addr link 0 or c-addr 0 0
   drop
   \ 0x46 emit .stack cr
   dup 0 = if over 10 dump else
      \ 0x44 emit .stack cr
      \ dispose word address
      swap drop
      \ check immediate flag
      dup c@ 0x80 and if 1 else -1 then
      \ xt
      swap
      link_addr cells +
      swap
   then
   rot debug
;

\
\ find .. default is 'last' dictionary
\
: find
   last -find ;

\ test word
: ftest
   accept
   32 word
   find ;

\ ===================================
\ number
\

: >number ( char -- num | -1 )
   \ one-digit conversion
   ;

: [char]
   literal literal 
   ] bl word dup h4. space 10 dump .stack ; immediate

\ : baka [char] a ;

