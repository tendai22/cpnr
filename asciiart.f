0x62 .ps
\ variable f 
\ variable x
\ variable y
\ variable a
\ variable b
\ variable c
\ variable d
\ variable ii
\ variable p
\ variable q
\ variable s
\ variable t
\ variable tmp
\ variable eol  0 eol !
\ variable out

: b0 0xf040 ;
: f [ ' dolit , b0 0 + , ] ;
: x [ ' dolit , b0 2 + , ] ;
: y [ ' dolit , b0 4 + , ] ;
: a [ ' dolit , b0 6 + , ] ;
: b [ ' dolit , b0 8 + , ] ;
: c [ ' dolit , b0 10 + , ] ;
: d [ '  dolit , b0 12 + , ] ;
: ii [ '  dolit , b0 14 + , ] ;
: p [  ' dolit , b0 16 + , ] ;
: q [ '  dolit , b0 18 + , ] ;
: s [ '  dolit , b0 20 + , ] ;
: t [ '  dolit , b0 22 + , ] ;
: tmp [ '  dolit , b0 24 + , ] ;
: eol [ '  dolit , b0 26 + , ] ;
0 eol !
: out [ '  dolit , b0 28 + , ] ;

: asciiart 
    50 f !
    0 eol !
    13 -12 do i y !
        30 -49 do i x !
        \ ." [" x . ." ," y . ." ]" cr
        x @ s->d 229 100 m*/ drop c ( 0x55 .ps ) !
        y @ s->d 416 100 m*/ drop d ( 0x56 .ps ) !
        c @ a !
        d @ b !
        0 ii !
        begin
            b @ f @ / q !
            b @ q @ f @ * - s !
            a @ dup * b @ dup * - f @ / c @ + tmp !
            2 a @ q @ * a @ s @ * f @ / + * d @ + b !
            tmp @ a !
            a @ f @ / p !
            b @ f @ / q !
            p @ dup * q @ dup * + t !
            \ if (t > 4) break;
            0 eol !
            ii @ ( 0x44 .ps ) drop
            t @ 4 > if 1 eol ! else
                ii dup @ 1+ swap !
                ii @ 16 < not if 1 eol ! then then
            eol @ not ( 0x41 .ps )
        while
        repeat
        \ char out
        ii @ 16 >= if 0x20 out ! else
        ii @ 10 <  if ii @ 0x30 + out ! else
                      ii @ 10 - 0x41 + out !
                    then then
        ( ." [" ) out @ emit ( ." ]" )
        loop
        cr
    loop
;
0x61 .ps
asciiart
