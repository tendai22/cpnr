//
// main.c ... cpnr main functions
// March 24, 2024  Norihiro Kumagai

#include <stdio.h>
#include <string.h>
#include "machine.h"

//
// forth interpreter initializer
//

void reset(context_t *cx)
{
    cx->pc = ROMSTART;
}

static void initialize_ctx(context_t *cx)
{
    cx->ip = 0;
    cx->wa = 0;
    cx->ca = 0;
    cx->rs = STACK_SIZE;
    cx->sp = STACK_SIZE;
    cx->ah = 0;
    cx->al = 0;
    reset(cx);      // initialize cx->pc
    memset(&cx->stack[0], 0, STACK_SIZE);
    memset(&cx->rstack[0], 0, STACK_SIZE);
    cx->tib = 0x4000;
}


//
//  counted-stringをC言語文字列にコピーする
//
char *str(mem_t *c_str)
{
    static char buf[256];
    char *p = (mem_t *)c_str;
    int n = *p;
    n = (n < 255) ? n : 255;
    memcpy (buf, p + 1, n);
    p[n] = '\0';
    return buf;
}

/*
void do_catch(context_t *cx)
{
    int result;
    result = setjmp(cx->env);    // no disclimination
}
*/
// do_abort
// If the flag is true, types out the last word interpreted, followed by the
// text. Also clears the user's stacks and returns control to the terminal. If
//false, takes no action.
/*
void do_abort(context_t *cx, const char *mes)
{
    char *p;
    if (tos(cx)) {
        p = str(MEMptr(cx, cx->var[LAST]));      // entry top, word name
        fprintf(stderr, "%s %s\n", p, mes);
    }
    longjmp(cx->env, tos(cx));
}

void do_halt(void)
{
    longjmp(cx->env, 99);
}
*/
// do_push

void do_push(context_t *cx, word_t value)
{
    cx->stack[--(cx->sp)] = value;
}

void do_dup(context_t *cx)
{
    do_push(cx, cx->stack[cx->sp]);
}

word_t tos(context_t *cx)
{
    return cx->stack[cx->sp];
}

// do_accept: read one line

// do_word: cut a word to top-of-dictionary

// do_find: find a entry whose name is the same
//   as top-of-directory
// (c-addr -- 0     (not found)
//            xt 1  (find, normak)
//            xt -1 (find, immediate))

// do_execute: invoke xt on tos
// (xt -- )
// xt actually is a code-address of the dict-entry

// innter interpreter
//

int main (int ac, char **av)
{
    // outer interpreter
    char buf[80];
    int n;
    context_t _ctx, *cx;

    // initialize ctx
    while (1) {
        cx = &_ctx;
        initialize_ctx(cx);
        monitor(cx);
    }
}

