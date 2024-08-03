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
