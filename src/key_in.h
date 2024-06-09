//
// key_in.h ... generic one key, raw mode key input interface
//

#if !defined(__KEY_IN_H)
#define __KEY_IN_H
extern void changemode(int dir);
extern int kbhit(void);
extern int key_in(void);
#endif //__KEY_IN_H