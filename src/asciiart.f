\
\ asciiart.f: delived from C code on 
\    `https://github.com/tgtakaoka/retro-bionic/tree/main`
\
variable f 
variable x
variable y
variable a
variable b
variable c
variable d
variable ii
variable p
variable q
variable s
variable t
variable tmp
variable eol  0 eol !
variable out

: asciiart 
    50 f !
    13 -12 do i y !
        30 -49 do i x !
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
            1 eol !
            ( ii @ 0x44 .ps drop )
            t @ 4 > if 0 eol ! else
                ii dup @ 1+ swap !
                ii @ 16 < not if 0 eol ! then then
            eol @ ( 0x41 .ps )
        while
        repeat
        \ char out
        ii @ 16 >= if 0x20 ( out ! ) else
        ii @ 10 <  if ii @ 0x30 + ( out ! ) else
                      ii @ 10 - 0x41 + ( out ! )
                    then then
        ( ." [" ) ( out @ ) emit ( ." ]" )
        loop
        cr
    loop
;
\ asciiart
