\
\ SPDX-License-Identifier: BSD-3-Clause
\ Copyright (c) 2024, Norihiro Kumagai
\
\ cold.f ... cold start routines
\
\ cmove ( from to count --- )
: init_dump DP_HEAD DP_ADDR END_ADDR DP_ADDR - cmove ;
: cold init_dump abort ;
' cold COLD_HEAD !