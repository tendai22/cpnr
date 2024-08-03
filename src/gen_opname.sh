#! /bin/sh
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2024, Norihiro Kumagai
#
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