//
// cfunc.c ... primitive (future overwritten ones)
// Apr 1, 2024 Norihiro Kumagai
//

#include <stdio.h>
#include <string.h>
#include "machine.h"

//
// get_instream
//
int get_instream(context_t *cx)
{
    int c;
    if (cx->p == 0 || cx->rest == 0)
        return 0;   // eof
    cx->rest --;
    c = *(cx->p)++;
    //fprintf(stderr, "(%c)", c);
    return c;
}

//
// do_accept: ( -- )
//
int do_accept(context_t *cx)
{
    static int outer_flag = 1;

    if (cx->p && cx->rest > 0) {
        return 0;
    }
    // now in-stream buffer emnty, refill it
    word_t *wp = &mem[S0_ADDR];
    char *buf = &mem[*wp];
    int size = 80, n;
    if (outer_flag) {
        if (gets_outer(buf, size - 1) == 0) {
            // outer data expires
            outer_flag = 0;
        }
    }
    if (outer_flag == 0) {
        // input from keyboard
        do_prompt(cx);
        if (fgets (buf, size - 1, stdin) == 0) {
            fprintf(stderr, "eof\n");
            return EOF;
        }
    }
    n = strlen(buf);
    while (0 < n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) {
        buf[--n] = ' ';
    }
    if (buf[n - 1] != ' ' && n < 79) {
        buf[n] = ' ';
        buf[n + 1] = '\0';
    }
    cx->p = buf;
    cx->rest = n;
    return 0;
}

//
// do_word: (delim -- addr)
//
void do_word(context_t *cx)
{
    char delim, *tib, *p, *here, *h0;
    word_t wp;
    int n, c;
    delim = tos(cx);
    here = h0 = &mem[mem[H_ADDR]];
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
        //fprintf(stderr, "[%c]", c);
    } while ((c = get_instream(cx)) != 0 && c != delim);
    *here = delim;   // trailing delim char, not counted
    *h0 = here - h0 - 1;        // count byte
    tos(cx) = cx->h;
}

// do_find: find a entry whose name is the same
//   as top-of-directory
// (c-addr -- 0     (not found)
//            xt 1  (find, normak)
//            xt -1 (find, immediate))
void do_find(context_t *cx)
{

}


void do_prompt(context_t *cx)
{
    fprintf(stderr, " ok\n");
}

// do_emit
void do_emit(context_t *cx, word_t w)
{
    fputc((w&0xff), stdout);
}

// do_create ( -- ) ... no stack operation
// パラメータ・フィールドのメモリを確保することなく、
// <name>の辞書エントリが作成される。その後、<name>が実行されると、
// <name>のパラメータ・フィールドの最初のバイトのアドレスがスタックに
// 残される。

// name string, link pointers are initialized, with user variable 
// 'h' alloted, 'last' remains to point top of this brand-new entry.

void do_create(context_t *cx)
{
    char *p, *q;
    word_t w;
    int count, n;
    // read a word to put it at the last of dictionary, pointed last, here
    do_push(cx, ' ');   // push a space as delimiter for do_word
    do_word(cx); 
    // check its name
    p = &mem[mem[H_ADDR]];
    count = *p;
    n = (count + 1) + ((count + 1) % 2);
    fprintf(stderr, "create: head = %04x, count = %d, n = %d\n", *h_wp, count, n);
    // put link pointer
    w = mem[LAST_ADDR];
    p += n;
    *((word_t *)p) = w;
    // uvar last update
    mem[LAST_ADDR] = mem[H_ADDR];
    // user h update
    p += 2; // p points parameter field.
    mem[H_ADDR] = p - &mem[0];    // allot'ed 
}

