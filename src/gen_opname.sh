#! /bin/sh
cat "$@" |
sed '
1i\
static const char *optable[256];\
void init_optable(void)\
{
$a\
}
/ropcode\[/!d
s/ + opcode_base//
s/ropcode\[/    optable[/
s/$/;/
'