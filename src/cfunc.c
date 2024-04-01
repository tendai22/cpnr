//
// cfunc.c ... primitive (future overwritten ones)
// Apr 1, 2024 Norihiro Kumagai
//

#include <stdio.h>
#include "machine.h"

//
// do_accept: (addr n+ -- addr n)
//
void do_accept(context_t *cx)
{
    char *buf = MEMptr(cx, cx->tib);
    int size = 80, n;
    fgets (buf, size - 1, stdin);
    n = strlen(buf);
    if (buf[n - 1] != ' ') && n < 80) {
        buf[n++] = ' ';
        buf[n] = '\0';
    }
    cx->stack[--cx->sp] = ptr2MEM(cx, buf);
    cx->stack[--cx->sp] = n;
}

void do_word(context_t *cx)
{
    char delim, *tib, *p;
    int n;
    delim = cx->stack[cx->sp];
    tib = MEMptr(cx, cx->tib);
     ( == ' ') {
        while ()
    }
    p = index(tib, delim);
    if (p == 0) {
        return 0;
    }
    // copy it tocounterd 

}
