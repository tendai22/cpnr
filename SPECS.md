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
 `>`|( n1 n2 --- t/f )|n1 > n2であれば真
 `>=`|( n1 n2 --- t/f )|n1 `>=` n2であれば真
 `>in`|( --- n )|行入力バッファのインデックス値のアドレス(ユーザ変数)
 `>mark`|( --- addr )|後方参照の開始
 `>r`|( n --- )R:( --- n )|データスタックからリターンスタックへの退避
 `>resolve`|( addr --- )|後方参照の解決
 `>rest`|( --- n )|rest of s0 buffer ;
 ?#eos|( c --- t/f )|space or nul
 ?#punct|( c --- t/f )|punctuation character?
 ?branch|( flag --- )|branch if flag is not zero
 ?csp|( --- )|保存しておいたスタックポインタと比較し、異なればエラー
 ?dup|( x -- 0/x x)|dup only if x is not zero
 ?error|( code --- )|exec verror if code is not zero
 ?stack|( --- )|スタックアンダーフローまたはオーバフローでエラー 
 ["]|\ ( --- c-addr )|文字列を切り出し辞書末尾に置く
 [char]|( --- c )|入力ストリーム次のワード先頭の文字コードを置く
 [compile]|( --- )|(P)入力ストリーム次のワードのxtをコンパイルする 
 [compile]|( --- ) 2nd definition, error check version
 [|( -- )|stateに0を代入
 ]|( -- )|stateに1を代入
 abort"|( n --- )|文字列をコンパイルする。nが非0の場合、文字列を出力し、定義の残りの実行をキャンセルしインタプリタを再実行する。
 abort|( --- )|スタックをクリアしてインタプリタを開始する
 abs|( n --- n' )|絶対値を返す。
 accept|( --- )|read a line to put it to s0
 again|( --- )|`begin ... again`ループ構造。 
 aligned|( addr -- addr2 )|CELL単位に伸ばした値を返す
 align|( --- )|dpをalignedする
 allot|( n --- )|dpをn進める
 and|( a b --- )|論理積
 base|( --- n )|基数を返す。
 begin|( --- )|`begin ... while ... repeat`ループ構造
 bitnot|( a --- a' )|ビット否定、1の補数。
 bl|( --- 32 )|スペース文字をスタックに置く
 boolean|( n -- ffff|0000 )|nをtrue/false値に変換 
 branch|( --- )|スレッドコード無条件ジャンプ
 bye|( --- )|テキストインタプリタを終了、呼び出したシステムに戻る
 c!|( c addr --- )|バイト書き込み
 c"|( --- c-addr )|文字列定数を置きアドレスを返す。
 c@|( addr --- c )|バイト読み出し
 cdump|( addr n -- )|バイトダンプ
 cells|( n -- m )|n x cell を返す。
 cell|( --- m )|1セルのバイト数を返す。
 cfa|dpを含むワードのCFAを返す
 char|( --- c )|入力ストリームから1ワード読み込み、先頭バイトを返す。
 cmove|( from to count --- )|バイト列転送
 compare|( c-addr1 u1 c-addr2 u2 -- n )|2個の文字列を比較。
 compile|( --- )|スレッドコード上で次のxtをコンパイルする
 constant|( n --- )|定数ワードを定義する
 count|( c-addr --- count addr+1 )|カウント付き文字列からカウントとアドレスに分ける
 create|( --- addr )|入力ストリームから1ワード読み込み辞書末尾に置く。PFAを返す
 cr|( --- )|CRLFを出力する。
 csp|( --- )|ユーザ変数、CSP_ADDRを返す。スタックポインタチェックに用いる。
 d+|( d1 d2 --- d3 )|倍長整数2個の加算
 d-|( d1 d2 --- d3 )|倍長整数2個の減算
 d.|( d1 --- )|倍長整数の印字
 d/|( ud1 n2 --- ud1/n2 )|倍長整数を単長整数で除算 same as `m/` 
 `d<`|( d1 d2 --- t/f )|倍長整数2個の比較
 d_dolit|( --- d1 )|倍長整数のリテラル命令
 dabs|( d1 --- d1' )|倍長整数の絶対値
 dd|( addr --- )|辞書エントリのディスアセンブル
 debug|( n --- )|デバッグレベルにnを設定する
 decimal|( --- )|基数を10にする
 depth|( -- +n )|スタック深さを返す
 dictdump|( top end last --- )|辞書をファイルに書き出す
 dicttop|( --- addr )|辞書先頭のアドレス(それを保持する変数アドレスと同じ)を返す。
 digit|( c base --- n2 flag )|文字cを基数baseで数値に変換する。flagが真の場合は変換成功で、n2が変換後の値、flagが偽の場合は変換失敗(元の文字が数値範囲外)。
 dliteral|( --- d1 )|compile double length integer
 dmax|( ud1 ud2 --- ud )|倍長整数2個のうち大きい方を残す
 dmin|( ud1 ud2 --- ud )|倍著整数2個のうち小さい方を残す
 dnegate|( d1 --- d2 )|倍長整数のマイナス値を返す
 `does>`|
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
 s2r|
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
