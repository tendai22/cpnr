# CPNR, A C Portable NarrowRoad Forth interpreter

本リポジトリは、C言語で動くForthインタプリタ"CPNR"を提供します。

## 内容物

Forth処理系、内部インタプリタと最低限の定義を実行するスレッドコードインタプリタをC言語で記述し、そのうえでコンパイル・実行できる外部インタプリタをForthのコロン定義で記述したものを含んでいます。

コロン定義のコンパイルが行う必要があるため、スレッドコードインタプリタは実質的に外部インタプリタを持ちます。`WORD`, `NUMBER`, `FIND`, `EXECUTE`, 辞書領域へのデータ書き込み(`,`(カンマ))などが必要になり、初期状態で実行可能なプリミティブは結構多いです。

外部インタプリタのコロン定義を記述するためには、`WORD`, `NUMBER`, `FIND`, `EXECUTE` を再定義します。再定義コロン定義がそろった時点で、プリミティブとして用意した`WORD`, `NUMBER`, `FIND`, `EXECUTE`などを削除して、残ったものだけをターゲットCPUの機械語で書き直すことで、ターゲットへの移植が完了するという考えです。

ソースコードライセンスはBSD-3clauseとします。

## 詳細

* [設計情報、「ワードリスト」](DESIGN.md)
* ソースコードは `src`の下にあります。
* ビルドは`build`の下で`make`コマンドを実行すればOKです。シェルスクリプト実行が必須ですので、Linux/Unix環境でお試しください。Bash前提ではないので、FreeBSD上でも実行できるはずです。
* 実行はコマンドを起動してください

```
NAME
    cpnr - A Forth interpreter-kit for porting new CPUs

SYNOPSIS
    cpnr [-c config-file] file ...

DESCRIPTION
    A forth interpreter writtend C.

    It start with very limited definitions, including inner/outer interpreter, 
    definitions of `:`(colon), `;` and some dictionary handling words.

    files are interpreted in turn, usually the first argument specify a meaningful forth word definitions.  After parsing all of the argument files, it enters an outer interpreter prompted with " OK".

    More detail description, available words are specified in SPECS.md'
```

* [ワード仕様](SPECS.md)

## 動機

特定のCPUに依らない実装を作る。Forth処理系の場合、内部インタプリタ(または、スレッドコード(threaded code)処理系<sup>1,</sup>)を仮想的なCPUの機械語として表現する。あるターゲットCPUの機械語を使い、この仮想的なCPUエミュレータを用意すれば、さまざまなCPU上で動かすことができる。効率はともかく。移植の立ち上がりはよいだろう。

仮想的CPUのエミュレータをC言語で記述すれば、高速なPC上でエミュレートも簡単だし、C言語が動くターゲットCPU上での動作も容易になる。C言語で開発できる環境ならForthを移植して使う必要もないやんかというツッコミも当然あるのだが、それはいったん脇に置いておく。

## きっかけ

「奥のほそ道」版 Forth 処理系を作成している。68000用を作ってきたが、いろいろしんどいことがありました。また、今後、さまざまなCPU上で動かしたいという欲が湧いてきました。

しんどかったこと2つですが、(1) `WORD`, `NUMBER`, 外部インタプリタをアセンブラで書くのがしんどい、Cで書きたい、と(2) 自作Forth処理系の立ち上げ過程(デバッグ)を楽したい、です。

#### 1. `WORD`, `NUMBER`, 外部インタプリタをアセンブリ言語で書くのがしんどい

この手の処理をアセンブリ言語で書くと100行越えになってしまいます。プログラム書き始めがC言語で、アセンブリ言語での大プログラム開発体力は貧弱です。`WORD`, `NUMBER`, `.`(ピリオド: 数字出力)はがちがちの文字列処理です。文字列処理はC言語で書き倒しているので手が勝手にコードを生み出してくれるのですが、アセンブリ言語で書くと、書くこと自体にも時間がかかります。

#### 2. デバッグ過程を楽したい

長いプログラムを書くと、些細なことそこここで引っかかってしまいます。デバッガを整備して進めてきたのですが、C言語で書いたプログラムのデバッグに比べると手間がかかります。

narrowroad-68kのときは Musashi エミュレータを改造してシングルステップ+レジスタダンプで68000CPUと機械語を勉強しながら進めました。ワード定義ができるようになると、今度はワード定義内のシングルステップとスタック+行バッファダンプが便利と分かりました。こちらは68000機械語で書くのですが、これもけっこうしんどい。

narrowroad-m68k開発は、外部インタプリタまで動く状態にして、いよいよForthワード定義で立ち上げるところまで来ましたが、ワード定義デバッグ環境もm68k機械語で作るのにも疲れてきました。ワード定義デバッグ環境もC言語で書けると楽だなぁ、と考えていました。

#### 3. C言語上で動くForth処理系が欲しい

Forth処理系をあるCPUに移植するに際し、周辺デバイスを動かすためには、仕様書を見ながらレジスタをぽちぽち叩くのですが、このやり方では動くところまで持ってゆくのが大変です。

CPUの周辺デバイス(SPI, I2C, フラッシュR/Wなど)を使うためのすぐ動くコードがC言語環境なら既に用意されています。最近の組み込み系CPUはベンダがCコンパイラと周辺デバイスをアクセスするためのライブラリを用意してくれています。提供されるサンプルコードを見ながらこのライブラリを呼び出すとすぐに動作させられます。このライブラリを呼び出して使えるようにForth処理系を作りたい、そのためには、C言語で記述するのが一番(おそらく唯一の方法)でしょうね。

Forthワードの中からC言語ライブラリを呼び出せるようにするには、実行系としてC言語の枠組みをそのまま使うことが必要です。要するに、main関数の中からForth処理系を呼び出すようにするのです。forth処理系は機械語プログラムに見えます。C言語のレジスタ扱いは尊重しておけば動くはず。そういう想定を持っています。

#### 参考: TILの内部インタプリタ表現を見る

そもそもの発端となった、TIL(Threaded Interpreter Language)の内部インタプリタ表現を参考に上げておきます。

コード自体は以下の通り。インストラクション幅は2バイト、ジャンプ命令は2+2バイトであることが分かる。

    Location   Mnemonic Instruction  Comment  

     0140       COLON   PSH I - RS  
     0142               WA - I  
     0144               JMP         ; Jump to NEXT  
     0146               0104          

     0100       SEMI    0102        ; Code address of SEMI  
     0102               POP RS - I  
     0104       NEXT    @I - WA  
     0106               I = I + 2  
     0108       RUN     @WA - CA  
     01OA               WA = WA + 2  
     010C               CA - PC  

     0050               7E          ; Dictionary  
     0052               XE          ; header  
     0054               LA          ; for EXECUTE  
     0056       EXECUTE 0058        ; Code address of  EXECUTE  
     0058               POP SP - WA
     005A               JMP         ; Jump to RUN
     005C               0108  

コロン定義のスレッドを実行するために必要なサポートルーチンは上記の5つだけです。

 |primitives|description|
 |--|--|
 |COLON|コロン定義の入り口の処理。IPをスタックに保持し、コロン定義スレッドの先頭のアドレスをIPに代入する。そのまま流れ込んだNEXTでスレッド先頭のアドレスから実行開始する。
 |SEMI|スレッド最後にこのルーチンのアドレスを置く。スタックからIPを戻し、NEXTになだれ込む。IPは、呼び出し元のスレッドの次のアドレスを指しているので、呼び出し元のスレッドの実行が継続する。
 |NEXT|IPが指す先の格納されたアドレスを取り出しそこにジャンプする(RUN)。
 |RUN|NEXT, EXECUTE共通の間接ジャンプルーチン。WAに取り出しておいた飛び先に飛び、WA, CAも更新しておく。WA, CAは機械語ワードの実行時に使用する。
 |EXECUTE|スタックトップにルーチンのアドレスが置いてある。そのアドレスのスレッドにジャンプする。


## Forth Interpreter

仮想CPUに加え、仮想デバイスとして、シリアルポート(IN/OUT)、ROM領域(起動時辞書とForthシステムを構成するプログラムを含む定義(テキスト表現)を起動時に読み込む)を持つ。RAMは読み込んだ定義をコンパイルする辞書領域と、ワークメモリ、スタック領域を保持する。シングルステップとデバッガも組み込む。

Linuxコマンドとして実現し、呼び出し時にForthシステム定義(テキスト表現)とデバッガスクリプトを引数で与える。起動するとモニタモードに入り、GOコマンドでインタプリタの実行を開始する。

シリアルポートエミュレーションは、キー入力の有無とデータレジスタレベルでシミュレーションする。端末ドライバはrawモードにして使う。エミュレータが暴走するとプロセスを殺してもキー入力がエコーバックしなくなることもある。その場合は、慌てず stty saneを実行しよう。

インタプリタ自体は、レジスタとワークの初期化の後に、外部インタプリタとして、辞書中の`QUIT`エントリのコードを`EXECUTE`する。


# 参考文献

1. Loeliger, R G., *Threaded interpretive languages.*, 1981 BYTE Publications Inc.

