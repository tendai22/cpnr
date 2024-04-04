//
// cfunc.c ... primitive (future overwritten ones)
// Apr 1, 2024 Norihiro Kumagai
//

#include <stdio.h>
#include "machine.h"

//
// get_instream
//
int get_instream(context_t *cx)
{
    if (cx->p == 0 || cx->rest == 0)
        return 0;   // eof
    cx->rest --;
    return *(cx->p)++
}

//
// do_accept: ( -- )
//
void do_accept(context_t *cx)
{
    static int outer_flag = true;

    if (cx->p && cx->rest > 0) {
        return;
    }
    // now in-stream buffer emnty, refill it    
    char *buf = MEMptr(cx, cx->s0);
    int size = 80, n;
    if (outer_flag) {
        if (gets_outer(buf, size - 1) != 0)
            ;
        else {
            // outer data expires
            outer_flag = 0;
        }
    }
    if (outer_flag == 0) {
        do_prompt(cx);
        fgets (buf, size - 1, stdin);
    }
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
    char delim, *tib, *p, *here, *h0;
    word_t wp;
    int n, c;
    delim = tos(cx);
    here = h0 = MEMptr(cx, cx->h);
    // skip if the first is space
    if ((c = get_instream(cx)) == 0) {
        // end-of-file
        tos(cx) = 0;
        return;
    } else if (c == delim) {
        while ((c = get_instream(cx)) == delim)
            ;   // skip delimiters
        if (c == 0) {
            tos(cx) = 0;
            return;
        }
    }
    // now c has neither delim nor eof
    here++;     // keep the first count char
    do {
        *here++ = c;
    } while ((c = get_instream(cx)) != 0 && c != delim);
    *here++ = delim;   // trailing delim char
    *h0 = here - h0;        // count byte
    tos(cx) = h0;
}
