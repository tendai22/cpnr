: words \ ( --- )
   last begin dup while
      \ dup h4. space
      dup h4. space
      dup c@ 0x1f and
      over 1+ swap ( 0x41 .ps ) type cr 
      lfa @ repeat
      drop
   ;
words
