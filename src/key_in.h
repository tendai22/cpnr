/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * key_in.h ... generic one key, raw mode key input interface
 * Copyright (c) 2024, Norihiro Kumagai
 */

#if !defined(__KEY_IN_H)
#define __KEY_IN_H
extern void changemode(int dir);
extern int kbhit(void);
extern int key_in(void);
#endif //__KEY_IN_H