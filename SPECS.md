 # narrowForth v0.92dev ワード一覧
 
 ||||
 |--|--|--|
 !|( w addr --- )|w をアドレスaddrに書き込む(ワード書き込み)。
 !#p|( c -- )
 !=|( w1 w2 --- t/f )|等しくなければ真
 !csp|( --- )|スタックポインタの値をcspに保存、コンパイル時のチェック用
 #>|( --- addr u )|`<# ... #>` で変換した数字文字列のアドレスを返す。
 #a2i|( c --- n )|数字文字1桁を数に変換A-Z,a-zは16以上を表す
 #base|( --- +n )|`<# ... #>`変換の基数を返す。
 #decimal|( --- )|`<# ... #>`変換の基数を10とする。
 #dec|( --- )|`<# ... #>`変換の基数を10とする。
 #field|( --- n )|`<# ... #>`変換の表示桁数を返す。
 #hex|( --- )|`<# ... #>`変換の基数を16とする。
 #i--|( --- )|`<# ... #>`変換のバッファ文字インデックスを1減らす。
 #i|( --- n )|`<# ... #>`変換のバッファ文字インデックス。
 #nb|( --- addr )|`<# ... #>`変換のバッファ先頭(here + 64 に確保している)
 #np|( --- addr )|`<# ... #>`変換の文字ポインタ
 #s|( u -- )|スタックトップの数値を最後まで変換する
 #|( d -- n/10 )|スタックトップの数値を1桁だけ変換し、スタックトップを1/#baseする
 '|( --- xt )|tick(quote)、入力ストリームの次のワードを辞書検索し、xtを返す
 (?error)|( flag n --- )|エラー発生を宣言してテキストインタプリタを最初からやり直す
 (abort)|( --- )|テキストインタプリタ再起動、データスタックを初期化、プロンプトを出力し`quit`に飛ぶ
 (do)|( limit index -- )<br> ( R: -- index limit )|doの下請け処理
 (does)|does>の下請け処理
 (error)|( w --- )|warning: -1: abort, 0: no descriptive, 1: descriptive
 (loop)|( delta -- )|loop, +loopの下請け処理
 (number)|( d1 addr1 --- d2 addr2 )|addr1から数字をスキャンしてd1を倍長整数d2に変換する。addr2は非数字文字のアドレス。
 (post-loop)||loopの後処理ワード。リターンスタックを戻す。
 `*`|( u1 u2 --- u )|mul
 +|( u1 u2 --- u )add
 +loop|( n --- )|`do ... +loop`構造。スタックの増分だけインデックスを増やす。 
 +rsp|( n --- )| rpsにnを足す。
 ,|( w --- )|スタックトップを辞書末尾(here)にコンパイルする
 -|( u1 u2 --- u )|u1 - u2をスタックトップに置く。
 -dup|dup if dup then ;
 -find|( c-addr -- c-addr 0 |<br> xt c 1 | xt c -1 )|辞書検索。lastから検索する。見つからなければ、次のnumberに向けてaddrを残す。immediateであれば1、通常ワードであれば-1を置く。
 ."|( --- )|`." ..."`、文字列出力
 .(|( --- )|`.( ...)`、コンパイル時にテキスト出力
 .cs|( c-addr --- )|count文字列出力
 .hd|( --- )|hereバッファの出力
 .ps|( c --- )|スタック出力、先頭に文字cをemitする。
 .stack|( --- )|データスタック出力。
 .|( n --- )|数値出力
 /|( n1 n2 --- n )|除算、n1 / n2
 /mod|( n1 n2 --- n )|剰余、n1 % n2
 0<=|( n --- t/f )|0より小さいか等しければ真
 0<|( n --- t/f )|0より小さければ真
 0=|( n --- t/f )|0と等しければ真
 0>=|( n --- t/f )|0より大きいか等しければ真
 0>|( n --- t/f )|0より大きければ真
 1+|( n --- n+1 )|1増やす
 1-|( n --- n-1 )|1減らす
 2drop|( n n --- )|2要素drop
 2dup|( n1 n2 --- n1 n2 n1 n2 )|2要素dup
 2swap|( n1 n2 n3 n4 --- n3 n4 n1 n2 )|2要素入替
 :|( --- )|コロン定義開始
 ;|( --- )|コロン定義終了
 ;;|( --- )|コロン定義終了
 `<#`|( d --- d )|数値出力変換開始
 `<=`|( n1 n2 --- t/f )|n1 <= n2であれば真
 `<mark`|( --- n )|前方参照のマーク
 `<resolve`|( n --- )|前方参照の解決
 `<`|( n1 n2 --- t/f )| `n1 < n2`であれば真  
 =whitespace|( char -- flag )|空白類文字であれば真
 =|( n1 n2 --- t/f )|n1 == n2であれば真
 >|( n1 n2 --- t/f )|n1 > n2であれば真
 >=|( n1 n2 --- t/f )|n1 >= n2であれば真
 >in|( --- n )|行入力バッファのインデックス値のアドレス(ユーザ変数)
 >mark|( --- n )
 >r s2r
 >resolve|here swap ! ;
 >rest|\ ( --- n ) rest of s0 buffer ;
 ?#eos|\ space or nul
 ?#punct|\ ( c --- flag )
 ?branch qbranch
 ?csp| ---
 ?dup|( x -- 0|x x)
 ?error
 ?exec|\ --
 ?stack| 
 @ fetch
 ["]|\ ( --- c-addr )
 [char]|
 [compile] ccompile immediate
 [compile]|( --- ) 2nd definition, error check version
 [|( -- ) 0 state ! ; immediate
 ]|( -- ) 1 state ! ;
 abort"| 
 abort|
 abs|dup msb and if 0 swap - then ;
 accept|\ ( --- ) read a line to put it to s0
 again|\ ( -- )
 aligned|( addr -- addr2 )
 align| 
 allot|dp @ + dp ! ;
 and
 base|BASE_ADDR @ ;
 begin|
 bitnot| 
 bl|32 ;
 boolean|( n -- ffff|0000 ) 
 branch
 bye
 c! cdepo
 c"( --- c-addr )... string constant with counted string
 c@ cfetch
 call
 cdump|( addr n -- ) \ simple dump
 cells|( n -- m )
 cell|
 cfa|lfa cell + ;
 char|bl word 1+ c@ ; immediate
 cmove|( from to count --- )
 compare|( c-addr1 u1 c-addr2 u2 -- n )
 compile|r> dup cell + >r @ , ;
 constant|create , does> @ ;
 count|( c-addr --- count addr+1 )
 create|
 cr|
 csp|CSP_ADDR ;
 d+ dadd
 d-|dnegate d+ ;
 d.|2dup dabs <# #s sign #> type drop drop space ;
 d/|\ ( ud1 n2 --- ud1/n2 ) ... same as `m/` 
 d< dlt
 d_dolit
 dabs|
 dd
 debug|DEBUG_ADDR ! ;
 decimal|10 BASE_ADDR ! ;
 depth|( -- +n )
 dictdump
 dicttop|DICTTOP_HEAD ;
 digit|( c base --- n2 flag )
 dliteral|( d --- ) ... compile double length integer
 dmax|( ud1 ud2 --- ud )
 dmin|( ud1 ud2 --- ud )
 dnegate|
 does>|
 dolit
 do|
 dp|DP_ADDR ;
 drop
 dump
 dump|( addr n -- ) \ simple dump
 dup 
 else|vBRANCH @ , >mark swap >resolve ; immediate
 emit
 entry_name|( entry -- )
 exch
 execute
 exec|vexecute execute ;
 exit|SEMI_HEAD @ , ; immediate
 false|( --- 0 )
 fill|( addr n c -- )
 find|
 getline
 getline|( n addr --- )
 h++|here c@ 1+ here c! ;
 h2.|print hex number
 h4.|print hex number
 halt
 here|dp @ ;
 hex|16 BASE_ADDR ! ;
 hold|( c -- ) \ append a char to nbuf
 hptr|here dup c@ + 1+ ;
 h|dp ;
 i2a|( n -- c )
 if|vQBRANCH @ , >mark ; immediate
 immediate|last c@ 0x80 or last c! ;
 in_p|( -- addr )
 in_rest|( -- n )
 inc_p|( n -- )
 interpret|
 i|( R: index limit ret-addr)
 j|
 kbhit
 key
 last|LAST_ADDR @ ;
 leave|
 lfa|
 literal|( n --- ) ... compile literal instruction
 lnum
 loop|limit -- limit if loop remains | none if loop exits)
 m*/ mmuldiv
 m+ madd
 m/mod|( ud1 n2 --- reminder udq )
 m/|( ud1 n2 --- ud )
 max|2dup - msb and if swap then drop ;
 message|
 min|2dup - msb and not if swap then drop ;
 mod|/mod drop ;
 negate|( n -- 0000|ffff )
 nip|( x1 x2 -- x2 )
 nop
 not
 number|\ addr -- d
 or
 outer
 over
 pfa|cfa cell + ;
 pick|( +n -- x )
 quit|
 r1!||store a word in index
 r1@||
 r2@||index in a word execution
 r3@||index in a word execution
 r> r2s
 repeat|( -- )
 rot
 rp! rsp_reset
 rsp
 s"|( --- count addr )... string constant for `type`
 s->d|( n -- d ) ... sign extension
 s0|S0_HEAD @ ;
 s_dolit
 sign|( n xx xx - n xx xx ) \ print '-' if n is minus
 sp! sp_reset
 sp@ sp_at
 spaces|( n --- )|1 do bl emit loop ;
 space|32 emit ;
 state|STATE_ADDR ;
 strict|( --- addr )|変数strictのアドレスを返す。インタプリタ実行時に都度スタックあふれチェック句をするかどうかを決める。
 strlen|( addr --- n )|ヌル終端文字列の長さを返す。
 swap
 then|>resolve ; immediate
 trap
 true|-1 ;
 type|( addr u -- ) \ print a string
 u* umul
 unloop|
 until|( flag -- )
 variable|create cell allot does> ; 
 while|( f -- )
 word|( delim -- addr|0 )
 xor
