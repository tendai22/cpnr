# Design memo

## 内部インタプリタ設計アイディア

内部インタプリタがスレッドコードを実行できるようにして、Forth処理系自体のほぼすべてをスレッドコードとして記述する。

間接コードスレッドとする。コードフィールドは、間接ジャンプのとび先１ワードのみ。CODEワードの場合は、".+2"(現在番地の次のワードへ飛ぶ)、コロン定義の場合は、M_COLON + M_NEXTエントリへ飛ぶ。パラメータフィールドのアドレス列の末尾はSEMIのコードフィールドを指す。

最初に存在するものは、ワードを定義するワードだけである。具体的には、

    @(at-mark) !(exclamation)       # memory operation
    : ; VARIABLE CREATE             # defining words
    + * -                           # alithmetic operations
    HERE ALLOT ,(comma) '(quote)    # dictionary operations
    LITERAL BRA BNZ                 # literal and branchs
    <MARK <RESOLVE >MARK >RESOLVE   # branch operand resolution
    COLON SEMI NEXT RUN             # inner interpreter
    WORD NUMBER FIND EXECUTE        # outer interpreter

これらのワードは全て処理実体をC言語関数として記述する。かつ、1仮想機械語命令1関数で用意する。その1命令だけを実行する辞書エントリを用意する。

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

```
    1100 012B       HEAD "+"
    1102 10F8       LINK e_0003 //link to previous entry
    1104 1106       DW .+2      //points 1 word later
    1106 c020       M_ADD       //machine code 'ADD'
    1108 C002       M_NEXT
```

* ワードBRA(スレッド内無条件ジャンプ)は以下の通り  
  機械語 M_BRA はIPを更新するが、PC自体は一つ進む。次の機械語命令としてM_NEXTを置く。

```
    110A 0342 5241  HEAD "BRA"
    110C 1100       LINK e_0004
    110E 1110       DW .+2
    1110 C006       M_BRA
    1112 C002       M_NEXT
```

```
    1114 0342 4E45  HEAD "BNE"
    1118 110A       LINK e_0005
    111E 1120       DW .+2
    1120 C005       M_BNE
    1122 C002       M_NEXT
```

機械語命令としてのPC手繰りと、スレッド実行としてのIP手繰りを区別する。PC手繰り中はIPは動かない。IPはCOLON, NEXT, SEMI, BRA, BNEで動かす。

## メモリマップ・システム構成

オールRAM、立ち上げ用初期辞書エントリをバイナリファイル読み込み後(実運用では「初期化されたデータ領域(.data)」を使用予定)、インタプリタを起動。

辞書生成のためのアセンブラを用意する。

そのアセンブラコードを出力する辞書コンパイラ`makedict.sh`を用意する。narrowroad-m68kから持ってきた。

## 辞書コンパイラ


    word <name>
        ...
        endword

または、

    code <name>
        endcode
    code <puncname> <name>
        endcode

に加えて、

    opcode <name>
    opcose <puncname> <name>

も受け入れる。`opcode`ディレクティブは、仮想機械の機械語1命令を実行するワードを定義する。

今回は、code定義なし、opcode定義でないものはForthコードをコンパイルして辞書を生成する。
`:`, `;`, `VARIABLE`は機械語1命令で、opcodeエントリで定義する。

## アセンブラ

[asxxxx](https://shop-pdp.net/ashtml/)パッケージをざっと眺めてみたが、思っていたより面倒くさそう。命令は表を書けばよさそうだが、オペランド解釈部分のCソースコードが頭に入ってこない。理解して使いこなすまでに時間がかかりそう。おじげついてしまっている。

FORTH用のアセンブラは、

* 各ルーチンのサイズ(命令数)は小さい。
* スレッドコードにアドレスを並べるので、シンボル定義・解決は欲しい。
* 今回の場合、命令数は少ない、1 Forthプリミティブ1命令のレベルなので。
* 機械語エントリ(CODEエントリ)のパラメータフィールドは２命令(対象オペレーションを行う命令1つとNEXT命令の２つ)、なので、
* 機械語ルーチン内で分岐はない、多分。

なので、シンボル定義・解決と、命令名から1ワード生成するawkスクリプトでいいだろう。

* シンボル操作は連想配列を用いる。2パス方式。1パス目でアドレス勘定・コード生成ののち、
そのファイルに対して2パス目を掛ける。
* 辞書ヘッダ生成、リンク生成のマクロを用意する(narrowroad-m68kで使ったものが使えるはず)

### ディレクティブ

 |directive|description|
 |--|--|
 .head|ヘッダ文字列を生成する。ワード境界でそろえる。<br>後ろに空白最低1文字置く。奇数文字の場合2バイト空白ということ。<br>先頭1バイトは文字列長さだが、上位3ビットはフラグ。<br>MSBがprecedence/即値ワードを示す。
 .dw|ワード定数。スレッドコードのコンパイル、リンクアドレス置きに使う。
 m_xxxx|機械語命令、`machine_code`関数のswitch文に対応コードが置かれている。
 .org|開始アドレスを指定する。

 