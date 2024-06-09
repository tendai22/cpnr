
: expect \ addr n ---
   over +
   over
   do 
      key
      dup 8 = over 0x7f = or
      if
         drop     \ drop the char
         dup i =  \ top of line?
         dup if 7 emit then
         r> 2 - + >r
         8 emit 32 emit 8
      else
         dup 0x0d = over 0x0a = or over 0x04 = or
         if
            drop 
            bl       \ as echo-hack char 
            0        \ as end-of-buffer char
         else dup
         then
         i over not if leave then
         c!          \ leave clears i, so pushed i
                     \ before it is lost.
      then
      0 i 1+ c!   \ ASCII nul for guard
      emit
   loop
   drop
   ;
: aho s0 80 expect s0 10 cdump ;
