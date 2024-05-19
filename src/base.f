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
: PAD_ADDR      0x4014 ;
: IN_ADDR       0x4016 ;

\ debug
: debug DEBUG_ADDR ! ;

\ base
: base BASE_ADDR @ ;
10 BASE_ADDR !

\ state
: state STATE_ADDR ;
0 state !

\ signbit
: signbit 0x8000 ;

\ cells
: cells 2 ;

\
\ simple ones
\
: 1+ 1 + ;
: 1- 1 - ;

\
\ dictionary/compilation
\

\ here/allot/last/immediate
: here H_ADDR @ ;
: allot H_ADDR @ + H_ADDR ! ;
: last LAST_ADDR @ ;
: immediate last c@ 0x80 or last c! ;
: , ( comma ) here ! cells allot ;
: ] ( -- ) 1 state ! ; immediate
: [ ( -- ) 0 state ! ; immediate

\ lfa, link_addr ( addr -- link-addr )
: lfa
   dup c@      \ addr c
   31 and      \ addr n (= c&0x1f)
   dup 1 and   \ addr n (n&1)
   cells + +   \ addr (n +(n&1)+2)
   + ;

\ code_addr ( addr -- code-addr )
: cfa 
   lfa cells + ;

\ create
: create
   32 word drop
   last              \ last
   here lfa    \ last link_pos
   dup rot swap      \ link_pos last link_pos
   !                 \ STAR(link_pos) = last 
   here LAST_ADDR !  \ link_pos
   2 cells * + H_ADDR !
   ;

\
\ does>
\

: (does)
   last lfa cells +  \ code_addr
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
: 0> 0 > ;
: 0>= 0 < not ;
: 0< 0 < ;
: 0<= 0 > not ;
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
: until \ ( flag -- )
   compile ?branch <resolve 
   ; immediate
: again \ ( -- )
   compile branch <resolve 
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

\ ========================================
\ double length integer
\
\ opcode d+  ( ud1 ud2 --- ud )
\ opcode m+  ( ud1 n2 --- ud )
\ opcode m*/ ( ud1 n2 n3 --- ud (= ud1*n2/n3))

: 2drop drop drop ;

: 2swap ( n1 n2 n3 n4 --- n3 n4 n1 n2 )
   >r    ( n1 n2 n3 )
   rot rot ( n3 n1 n2 )
   r>    ( n3 n1 n2 n4 )
   rot rot ( n3 n4 n1 n2 )
   ;

: m/ \ ( ud1 n2 --- ud )
    1 swap ( ud1 1 n2 )
    m*/    ( ud1/n2 )
    ;

: bitnot 
   negate 1- ;

: dnegate
   bitnot swap bitnot swap
   1 m+
   ;

: dabs
   dup 0x8000 and if \ negative
      dnegate then ;

: m/mod \ ( ud1 n2 --- reminder udq )
   2 pick 2 pick 2 pick ( ud1 n2 ud1 n2 )
   m/  ( ud1 n2 ud1/n2 )
   2dup 
   >r >r ( save ud1/n2 )
   rot ( ud1 ud1/n2 n2 )
   1   ( ud1 ud1/n2 n2 1 )
   m*/ ( ud1 ud1/n2*n2 )
   dnegate ( ud1 -ud1/n2*n2)
   d+  ( dmod )
   drop ( mod ) \ single length reminder
   r> r> ( mod ud1/n2 )
   ;

: s->d \ ( n -- d ) ... sign extension
   dup signbit and if \ negative
      0xffff
   else \ positive
      0
   then
   ;

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
16  #field_addr !

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

: # ( d -- n/10 )
   #base m/mod  ( d -- d%10 d/10 ) 
   rot         ( d/10 d%10 )
   i2a #np c!
   #i-- ;

: #s ( u -- )
   begin # 2dup or not until ;

: #> ( -- ) \ return string for `TYPE` 
   drop drop #nb #i + 1+ #field #i - ;

: hold  ( c -- ) \ append a char to nbuf
   #np c! #i-- ;

: type ( addr u -- ) \ print a string
   swap 1- swap 1 do dup i + c@ emit loop drop ;

: sign ( n xx xx - n xx xx ) \ print '-' if n is minus
   2 pick signbit and if 45 #np c! #i-- then ;

: abs dup signbit and if 0 swap - then ;

: d. 2dup dabs <# #s sign #> type drop drop space ;

: . s->d d. ;

: #hex 16 #base_addr ! ;
: #decimal 10 #base_addr ! ;
: #dec 10 #base_addr ! ;

: h2.  \ print hex number
  0 <# #hex # # #> type ;

: h4.  \ print hex number
  0 <# #hex # # # # #> type ;

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
: pad PAD_ADDR @ ;
: in_p ( -- addr )
   pad dup c@ + 1+ ;
: inc_p ( n -- )
   pad dup c@ 1+ swap c! ;
: in_rest ( -- n )
   127 pad c@ - ;

: >in IN_ADDR ;

variable outer_flag
1 outer_flag !

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

\ input buffer, s0, 128bytes
\ top(*s0) holds the current index
\ accept ... primitive ( -- ), check s0 c@
: =whitespace ( char -- flag )
   dup  13 = 
   over 10 = or
   over 8 = or
   swap drop ;

\ : w_getch 
\   pad >in @ + c@ \ dup h2. space    \ pad[i]
\ ;
\ : w_i++ 
\   >in dup @ 1+ swap ! ;

\ : accept ( -- )
\   0 pad c!              \ i = 1
\   127 1 do
\      w_getch 
\      dup 0 = if
\         leave
\      else dup =whitespace if
\         leave
\      else dup 0 = if
\         leave
\      else dup 0 > if       \ valid char
\         in_p c!        \ *p = c
\         inc_p          \ i++ 
\      then then then then
\   loop
\   bl in_p c!
\   inc_p 
\   1 >in ! 
\   drop ;

\ test getline
: gtest 127 pad 1+ getline ;

: abort 0x2a emit halt ;

: strlen \ ( addr --- n )
   dup
   begin 
      dup c@
   while \ not nul char
      1+
   repeat
   \ top end
   swap -
;

\ : aho 127 pad 1 + getline pad 1 + 16 dump ;

: accept \ ( --- ) read a line to put it to pad
   127 pad 1+ getline
   not if 0 pad c! abort then
   \ now got a line on pad
   pad dup 1+ strlen +   \ &pad[strlen]
   \ eliminate trailing cr/lf
   pad 1+ swap do 
      i c@
      dup 13 = if 0 i c! else
      dup 10 = if 0 i c!
               else leave
      then then
      drop
      -1 +loop
   pad dup 1+ strlen swap c!
   1 >in !
   ;

: atest accept pad 16 dump 0 pad c! ;

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
: word ( delim -- addr|0 )
   0              \ ( -- delim flag ) ... flag show if it has started to store chars
   0 here c!      \ clear here[0]
   pad dup dup c@ 1+ + swap ( 0x30 .ps ) \  limit-index
   >in @ +        \ start-index
   ( 0x41 .ps )
   over over <= if drop drop drop drop 0 0x42 .ps exit then
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
   drop drop here c@ if here else 0 then ( 0x43 .ps )
   ( .hd ) ;

\ test word for accept-word
: wtest accept begin bl word c@ while .hd repeat ;

: >rest \ ( --- n ) rest of pad buffer ;
   pad c@ 1+ >in @ ( 0x31 .ps ) - ;

: wwtest
   begin 
      begin >rest 0x48 .ps 0> not while 0x23 emit space accept repeat
      begin bl word while repeat
   0 until ;

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
: aligned \ ( addr -- addr2 )
   cells + 1- cells / cells *
   ;

: align 
   0x41 .ps here aligned H_ADDR 0x42 .ps ! ;


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

\ lfa ( entry -- link )
\ : link_addr
\    dup c@ 0x1f and   \ n = *entry & 0x1f
\    dup 1 and + 2 + +
\ ;

\ prev_link ( entry -- prev-entry )
: prev_link
   lfa @
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
   swap      \ c-addr 0 last
   ( 0x40 emit .stack )
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
   dup 0 = if ( over 10 dump ) else
      \ 0x44 emit .stack cr
      \ dispose word address
      swap drop
      \ check immediate flag
      dup c@ 0x80 and if 1 else -1 then
      \ xt
      swap
      lfa cells +
      swap
   then
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

\ : [.ps] 0x58 .ps cr ; immediate

: ' \ comma ... find address of next string in dictionary
   compile literal
   bl word ( 0x58 .ps .hd cr )
   find 
   not if abort then 
   ( 0x58 .ps )
   , ( last 16 dump ) 
   ; immediate

\ : baka ' getline ;

: [char]
    compile literal 
    bl word 1+ c@ , 
    ; immediate

\ : aho [char] a ;

\
\ string manipulation
\

\ 1 debug

: ["] \ ( --- c-addr )
    \ leave the address of a counted-string, on where 'here'
   compile sliteral
   [char] " word ( dup 16 dump )
   c@ 1+ allot  ( here h4. 0x41 .ps )
   align ( here h4. cr )
   ; immediate

: count \ ( c-addr --- count addr+1 )
   dup 1+ swap c@ ;

: ." \ "
   [compile] ["]
   ' count ,
   ' type ,
   ; immediate

\ : aho ." baka" ;

\ : [compile] \ compile a word (even if it is immediate)
\    bl word find not if here count type ." not found" abort then ,
\    ; immediate

\ : baka ' + , 1 , ; 
\ : baka [char] x ;
\ : baka [compile] [ ;

\
\ rest of double length number alithmetics
\
: d- dnegate d+ ;
: d/ \ ( ud1 n2 --- ud1/n2 ) ... same as `m/` 
  m/ ;
: dmax \ ( ud1 ud2 --- ud )
   3 pick 3 pick 3 pick 3 pick ( ud1 ud2 ud1 ud2 ) 
   d<      if 2swap then 2drop ;
: dmin \ ( ud1 ud2 --- ud )
   3 pick 3 pick 3 pick 3 pick ( ud1 ud2 ud1 ud2 ) 
   d< not if 2swap then 2drop ;


: #a2i \ ( c --- n )
   dup 
   48 - dup 0< if else        \ \0 ... '0'
   10 - dup 0< if 10 + else   \ '0' ... '9'
    7 - dup 0< if else        \ ...
   26 - dup 0< if 36 + else   \ 'A' ... 'Z'
    6 - dup 0< if else        \ ... 
   26 - dup 0< if 36 + else   \ 'a' ... 'z'
                  drop -1
   then then then then then then
   swap drop
;

: ?#punct \ ( c --- flag )
   dup 0x2c = if -1 else \ comma else
   dup 0x2e = if -1 else \ period
   dup 0x2f = if -1 else \ slash
   dup 0x2a = if -1 else \ aster
   dup 0x3a = if -1 else \ colon
    0
   then then then then then
   swap drop 
;

variable dpl
0 dpl !
variable #base
10 #base !

: digit \ ( c base --- n2 flag )
   swap     ( base c )
   dup 
   48 - dup 0< if else        \ \0 ... '0'
   10 - dup 0< if 10 + else   \ '0' ... '9'
    7 - dup 0< if else        \ ...
   26 - dup 0< if 36 + else   \ 'A' ... 'Z'
    6 - dup 0< if else        \ ... 
   26 - dup 0< if 36 + else   \ 'a' ... 'z'
                  drop -1
   then then then then then then
   ( 0x33 .ps )   \ ( base c n2 )
   swap drop dup \ ( base n2 n2 )
   ( 0x34 .ps )   \ ( base n2 n2)
   0< if drop drop 0 else
   swap over ( 0x35 .ps ) <= if \ ( n2 base n2 )
      drop 0 
   else
      -1 
   then then
   ( 0x36 .ps )
   ;

\ (number) .. support word for `number`
\ start scan from addr+1, for `number`s convinience
\
: (number) \ ( d1 addr1 --- d2 addr2 )
   \ check if '0x'
   \ base #base ! 
   dup 1+ c@ 0x30 ( 0x39 .ps ) =  \ ( d1 addr1 flag1 )
   over 2 + c@ 0x78 ( 0x38 .ps ) = and ( 0x37 .ps ) \ ( d1 addr1 flag1&flag2 )
   if 16 #base ! 2 + else base #base ! then
   ( 0x36 .ps ) 
   begin
      1+ dup >r \ Save addr1+1, address of the first digit, on
            \ return stack.
      c@    \ Get a digit
      #base @  \ Get the current base
      digit \ A primitive. (c n1 -- n2 tf or ff)
            \ Convert the character c according to base n1 to
            \ a binary number n2 with a true flag on top of
            \ stack. If the digit is an invalid character, only
            \ a false flag is left on stack.
   while    \ Successful conversion, accumulate into d1.
      ( 0x42 .ps )
      swap  \ Get the high order part of d1 to the top.
      #base @ u* \ Multiply by base value
      drop  \ Drop the high order part of the product
      rot   \ Move the low order part of d1 to top of stack
      #base @ u* \ Multiply by base value
      ( 0x43 .ps )
      d+    \ Accumulate result into d1
      dpl @ 1+ \ See if DPL is other than -1
      if    \ DPL is not -1, a decimal point was encountered
         1 dpl @ + dpl ! \ Increment DPL, one more digit to right of
            \ decimal point
      then
      r> \ Pop addr1+1 back to convert the next digit.
   repeat \ If an invalid digit was found, exit the loop here.
         \ Otherwise repeat the conversion until the string is
         \ exhausted.
   r>    \ Pop return stack which contains the address of the first
         \ non-convertable digit, addr2.
   ;

\ test for (number)
 : nt 0 0 accept pad 0x40 .ps (number) 0 pad c! pad 16 dump ;

\ temporal stub
: ?error drop drop ;

: ?#eos \ space or nul
   dup bl = over 0 = or
   swap drop
;

\
\ number
\
: number \ addr -- d
   0 0 rot     \ Push two zero's on stack as the initial value
               \ of d. 
   dup 1+ c@   \ Get the first digit
   0x2d =      \ Is it a - sign?
   0x61 .ps 
   dup >r      \ Save the flag on return stack.
   if 1+ then  \ If the first digit is minus, increment addr
   -1          \ The initial value of DPL
   begin       \ Start the conversion process
      dpl !    \ Store the decimal point counter
      (number) \ Convert one digit after another until an 
               \ invalid char occurs. Result is accumulated
               \ into d.
      dup c@   \ Fetch the invalid digit
      ?#eos not   \ blank or nul?
   while       \ Not a blank, see if it is a decimal point
      dup c@   \ Get the digit again
      ?#punct not    \ Is it a punctuation?
      0 ?error \ Not a decimal point. It is an illegal 
               \ character for a number. Issue an error 
               \ message and quit.
      0        \ A decimal point was found. Set DPL to 0 the
               \ next time.
   repeat      \ Exit here if a blank was detected. Otherwise
               \ repeat the conversion process.
   drop        \ Discard addr on stack
   r>          \ Pop the flag of - sign back
   0x62 .ps
   if dnegate   \ Negate d if the first digit is a - sign.
   then
   ; \          All done. A double integer is on 

: nn accept pad 0x40 .ps number 0 pad c! pad 16 dump ;


\
\ interpret ... interpret words in the line input
\
: interpret
   begin
      bl word
   not while
      \ a valid word
      find
      if
         state @ <
         if cfa ,    \ compile it
         else
            cfa execute
         then
         \ ?stack
      else  \ not found, check number
         here
         number
         dpl @ 1+    \ dpl + 1
         if
            [compile] dliteral
         else
            drop
            [compile] literal
         then
         \ ?stack
      then
   repeat
   ;

\
\ quit ... outer interpreter main loop
\

: quit
   [compile] [ \ start interpretive state
   begin
      \ rp!
      cr
      127 s0 getline
      not if abort then \ stop interpreter when enter ^D
      \ interpret
      state @ 0=
      if
         ." ok"
      then
   again
   ;

: 2, \ ( d --- ) ... compile a double length integer
   swap ' literal , , ' literal , , ;
: aho 100000 0x44 .ps 2, ; immediate
: baka aho ;

last dd
