# Design Idea

内部インタプリタがスレッドコードを実行できるようにして、Forth処理系自体のほぼすべてをスレッドコードとして記述する。

最初に存在するものは、ワードを定義するワードだけである。具体的には、

    @(at-mark) !(exclamation)       # memory operation
    : ; VARIABLE CREATE             # defining words
    + * -                           # alithmetic operations
    HERE ALLOT ,(comma) '(quote)    # dictionary operations
    LITERAL BRA BNZ                 # literal and branchs
    <MARK <RESOLVE >MARK >RESOLVE   # branch operand resolution
    COLON SEMI NEXT RUN             # inner interpreter
    WORD NUMBER FIND EXECUTE        # outer interpreter

これらのワードは全て処理実態をC言語関数として記述する。

実行コンテキストは、以下の要素を持つ構造体とする。

    IP, WA, CA, RS, SP, PC, AH, AL: 16ビットレジスタ
    ver[]   # ユーザ変数を保持する配列。

まずここまでで作ってみよう。

 Register|Description  
 |--|--|
 IP|インストラクションレジスタ。現在実行中のセカンダリワードの中で次の命令のアドレスを保持する。
 WA|ワードアドレスレジスタ。現在実行中のキーワードのアドレス、または、現在のキーワードのボディ位置の最初のコードのアドレスを保持する。
 CA|コードアドレスレジスタ。
 RS|リターンスタックレジスタ
 SP|スタックポインタレジスタ
 PC|プロセッサのプログラムカウンタレジスタ
 AH,AL|アキュムレータ16bitレジスタ2本

考え直して、必要なCODEワード一つに機械語命令1つを割り当ててみた

 |word|instruction|description|
 |--|--|--|
 |c001|COLON|IPを保存してWAをIPにmovする
 |c002|NEXT|@IPをWAにmovして、IP +=2する(IPはスレッドの次のワードのアドレスを指す)。
 |c003|RUN|@WAをCAにmovして、WA +=2する。最後にCAをPCにmovする(現ワードCode Area番地にジャンプ(PC移動)する)
 |c004|SEMI|IPをリターンスタックから戻し、スレッドの次のワード実行に移る。

CODEワード定義は以下の通り(big endian)

* code fieldに .+2 を置く(次のワードの機械語から実行開始)
* 機械語の最後は NEXT 命令(スレッドの次のワードの実行)

    1100 012B       HEAD "+"
    1102 10F8       LINK e_0003 //link to previous entry
    1104 1106       DW .+2      //points 1 word later
    1106 c020       M_ADD       //machine code 'ADD'
    1108 C002       M_NEXT

* ワードBRA(スレッド内無条件ジャンプ)は以下の通り  
  機械語 M_BRA はIPを更新するが、PC自体は一つ進む。次の機械語命令としてM_NEXTを置く。

    110A 0342 5241  HEAD "BRA"
    110C 1100       LINK e_0004
    110E 1110       DW .+2
    1110 C006       M_BRA
    1112 C002       M_NEXT

    1114 0342 4E45  HEAD "BNE"
    1118 110A       LINK e_0005
    111E 1120       DW .+2
    1120 C005       M_BNE
    1122 C002       M_NEXT

機械語命令としてのPC手繰りと、スレッド実行としてのIP手繰りを区別する。PC手繰り中はIPは動かない。IPはCOLON, NEXT, SEMI, BRA, BNEで動かす。