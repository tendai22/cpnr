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
    memset(&cx->stack[0], 0, 2* STACK_SIZE);
    memset(&cx->rstack[0], 0, 2* STACK_SIZE);
    cx->s0 = S0_ADDR;
    cx->h = RAMSTART;
    cx->last = 0;
}

//
// 
//
static char **filenames;
static int filenames_length;
static FILE *fp;

void init_outer_buf(int ac, char **av)
{
    filenames = av;
}

int gets_outer(char *buf, int size)
{
    while (1) {
        if (filenames == 0 || filenames[0] == 0) {
            filenames = 0;
            buf[0] = 0;
            return 0;
        }
        if (fp == 0) {
            if ((fp = fopen(filenames[0], "r")) == 0) {
                fprintf(stderr, "gets_outer: cannot open %s, no file input any more\n", filenames[0]);
                filenames = 0;
                buf[0] = 0;
                return 0;
            }
            // now file input established
        }
        if (fgets(buf, size, fp) != 0) {
            fprintf(stderr, "gets_outer: read: %s\n", buf);
            return 1; // ok, read a line and return
        }
        // try to open next file
        fclose(fp);
        fp = 0;
        filenames++;
        // loop again, open next file and try to read the 1st line
    }
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



// do_push

void do_push(context_t *cx, word_t value)
{
    cx->stack[--(cx->sp)] = value;
}

void do_dup(context_t *cx)
{
    do_push(cx, cx->stack[cx->sp]);
}

void init_dict(void)
{
    static char *lower_hex = "0123456789abcdef";
    static char *upper_hex = "0123456789ABCDEF";
    char *filename = "dict.X", *p;
    FILE *fp = fopen(filename, "r");
    word_t addr = 0;

    if (fp == 0) {
        fprintf(stderr, "no dict file: %s\n", filename);
        return;
    }
    // read it
    int value;
    while ((c = fgetc(fp)) != EOF) {
        if (c == ' ')
            continue;
        if (c == '=') {
            // do address
            if ((value = fgethex(fp)) == -1)
                continue;
            if (value < 0 || MEMSIZE <= value)
                continue;
            addr = value;
        }
        if ((p = index(lower_hex, c)) || (p = index(upper_hex, c))) {
            ungetc(c, fp);
            value = fgethex(fp);

        }
    }


}

/*
word_t tos(context_t *cx)
{
    return cx->stack[cx->sp];
}
*/

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

    // init source file args
    init_outer_buf(ac - 1, av + 1);
    // initialize ctx
    while (1) {
        cx = &_ctx;
        initialize_ctx(cx);
        init_dict();
        if (monitor(cx) < 0)
            break;
    }
}

