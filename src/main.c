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

#define USER(n) (0x4000+(n))

static void initialize_ctx(context_t *cx)
{
    cx->ip = 0;
    cx->wa = 0;
    cx->ca = 0;
    cx->rs = RSTACK_END;
    cx->sp = DSTACK_END;
    cx->ah = 0;
    cx->al = 0;
    reset(cx);      // initialize cx->pc
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
    cx->sp -= 2;
    word_mem(cx->sp) = value;
}

void do_dup(context_t *cx)
{
    do_push(cx, word_mem(cx->sp));
}

//
// init_dict
//

// to_hex(int c)
// return its hex value if the char c is one of hex chars,
// otherwise return -1
static int to_hex(int c)
{
    static char *lower_hex = "0123456789abcdef";
    static char *upper_hex = "0123456789ABCDEF";
    const char *p;
    if ((p = index(lower_hex, c))) {
        return p - lower_hex;
    }
    if ((p = index(upper_hex, c))) {
        return p - upper_hex;
    }
    return -1;
}

//
// fgethex(fp)
// read hex string to convert it in positive hex value,
// otherwise return -1
static int fgethex(FILE *fp)
{
    int value = 0, c = -1, n = -1, valid_flag = 0;
    while ((c = fgetc(fp)) != EOF && (n = to_hex(c)) >= 0) {
        valid_flag = 1;
        value = value * 16 + n;
    }
    if (c != EOF && n < 0)
        ungetc(c, fp);
    if (valid_flag)
        return value;
    else 
        return -1;
}

void init_dict(context_t *cx)
{
    char *filename = "dict.X", *p;
    FILE *fp = fopen(filename, "r");
    int c, n, i;
    word_t addr = 0, *wp;

    if (fp == 0) {
        fprintf(stderr, "no dict file: %s\n", filename);
        return;
    }
    // read it
    int value, min = 0xffff, max = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c == ' ' || c == '\r' || c == '\n')
            continue;
        if (c == '=') {
            // do address
            if ((value = fgethex(fp)) == -1)
                continue;
            if (value < 0 || MEMSIZE <= value)
                continue;
            if (addr != value) {
                fprintf(stderr, "addr value = %04x\n", value);
                addr = value;
            }
            // min, max
            if (addr < min)
                min = addr;
            if (max < addr)
                max = addr;
        }
        if ((i = to_hex(c)) >= 0) {
            n = 1;
            value = i;
            while ((c = fgetc(fp)) != EOF && (i = to_hex(c)) >= 0) {
                value = value * 16 + i;
                n++;
            }
            if (c != EOF && i < 0)
                ungetc(c, fp);
            if (n > 2) {
                fprintf(stderr, "[%04X %04X]\n", addr, value);
                *((word_t *)&(mem[addr])) = value;
                addr += 2;
            } else {
                fprintf(stderr, "[%04X %02X]\n", addr, value);
                mem[addr] = value;
                addr++;
            }
        }
    }
    int first = 1;
    fprintf(stderr, "init_dict: min = %04x, max=%04x\n", min, max);
    for (int i = min; i <= max; i += 2) {
        if (first || (i % 16) == 0) {
            fprintf(stderr, "%04X: ", i);
            first = 0;
        }
        fprintf(stderr, "%04X ", *((word_t *)&(mem[i])));
        if ((i % 16) == 14)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
    // init user vars
    word_mem(LAST_ADDR) = word_mem(min + 4);
    word_mem(H_ADDR) = word_mem(min + 2);
    fprintf(stderr, "last: %04x, h: %04x\n", word_mem(LAST_ADDR), word_mem(H_ADDR));
}

// set xt to a user var
// name2xt ( -- xt), uses C arg 'name'
static int name2xt(context_t *cx, char *name)
{
    char *p;
    int n;
    word_t cstr_addr = word_mem(H_ADDR);
    // get 'name' entry address
    p = &mem[cstr_addr];
    *p++ = n = strlen(name);
    strncpy(p, name, n);
    p[n] = ' ';
    do_push(cx, cstr_addr);
    do_find(cx);
    if (do_pop(cx) == 0) {
        fprintf(stderr, "init_mem: %s: no entry, error\n", name);
        return -1;
    }
    return 0;
}

// initialize mem[] array, mainly user variables

static int init_mem(context_t *cx)
{
    mem_t *p;
    int flag = 0;
    word_mem(S0_ADDR) = DSTACK_END;  // s0 line buffer
    word_mem(STATE_ADDR) = 0;    // interpretive mode
    word_mem(BASE_ADDR) = 10;     // DECIMAL mode
    flag |= name2xt(cx, "halt");
    word_mem(HALT_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "colon");
    word_mem(COLON_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "semi");
    word_mem(SEMI_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "literal");
    word_mem(LITERAL_ADDR) = do_pop(cx);
    if (flag) {
        return -1;
    }
    fprintf(stderr, "init_mem: halt xt = %04X, semi xt = %04X\n", word_mem(HALT_ADDR), word_mem(SEMI_ADDR));
    return 0;
}
/*
word_t tos(context_t *cx)
{
    return cx->stack[cx->sp];
}
*/

// do_accept: read one line

// do_word: cut a word to top-of-dictionary



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
        init_dict(cx);
        if (init_mem(cx) < 0)
            break;
        if (monitor(cx) < 0)
            break;
    }
}

