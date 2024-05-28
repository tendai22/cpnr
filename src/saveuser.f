
\ dumpuser ... user area save buffer in dictionary
variable dumpuser END_ADDR DICTTOP_ADDR - cells + ( 0x41 .ps ) allot
: restoreuser dumpuser DICTTOP_ADDR END_ADDR DICTTOP_ADDR - cmove ;
DICTTOP_ADDR dumpuser END_ADDR DICTTOP_ADDR - cmove
