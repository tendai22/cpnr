//
// monitor.c:  cpu emulator execution and debugger
//

#include <stdio.h>
#include <setjmp.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "machine.h"
#include "user.h"

//
// optable
//
#include "opname.h"

const char *opcode_name(word_t mcode)
{
    static int count = 10;
    // relative m_jmp instruction
    if (mcode & 0x8000) {   // m_jmp relative jump
        return "m_jmp";
    }
    // normal opcode
    mcode -= OPCODE_BASE;
    if (mcode > sizeof optable / sizeof (const char *)) {
        if (count-- <= 0) {
            exit(1);
        }
        return "*toobig*";
    }
    return optable[mcode] ? optable[mcode] : "*undef*";
}

#define GO_CMD 1

static struct termios tb;
static int saved_flag = 0;

void raw_mode(void)
{
    struct termios tb_raw;
    tcgetattr(0, &tb);
    tb_raw = tb;
    saved_flag = 1;
    cfmakeraw(&tb_raw);
    tcsetattr(0, TCSANOW, &tb_raw);
}

void recover_mode(void)
{
    if (saved_flag)
        tcsetattr(0, TCSANOW, &tb);
}

int getcmd(void)
{
    int c, count = 0;
    raw_mode();
    while ((c = getchar()) == '.') {
        if (++count >= 3)
            break;
    }
    if (count >= 3) {
        recover_mode();
        return GO_CMD;
    }
    recover_mode();
    if (c == '\r')
        c = '\n';
    return c == 4 ? -1 : c;
}

int monitor (context_t *cx)
{
    int c, result;
    while (1) {
        // monitor prompt
        fprintf(stderr, "]");
        fflush(stderr);
        c = getcmd();
        if (c < 0) {
            fprintf(stderr, "\n");
            return -1;
        }
        if (c == '\n') {
            fprintf(stderr, "\n");
            continue;
        }
        if (c == GO_CMD) {
            if (do_mainloop(cx) < 0) {
                fprintf(stderr, "do_mainloop: over\n");
                return -1;
            }
        } else if (c == 'r') {
            fprintf(stderr, "\nreset done\n");
            reset(cx);
        } else {
            // not found
            fprintf(stderr, "monitor: bad command \"%c\"\n", c);
        }
    }
}

void print_stack(context_t *cx)
{
    int first;
    fprintf(stderr, "[");
    first = 1;
    for (word_t w = DSTACK_END - CELLS; w >= cx->sp; w -= CELLS) {
        if (first) {
            first = 0;
        } else {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "%04X", STAR(w));
    }
    fprintf(stderr, "] [");
    first = 1;
    for (word_t w = RSTACK_END - CELLS; w >= cx->rs; w -= CELLS) {
        if (first) {
            first = 0;
        } else {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "%04X", STAR(w));
    }
    fprintf(stderr, "]");
}

void print_next(context_t *cx, word_t xt)
{
    char *p = &mem[entry_head(cx, xt)];
    fprintf(stderr, ":%04X %04X [%-12.*s] ", cx->ip, xt, (*p & 0x1f), (p+1));
    print_stack(cx);
    fprintf(stderr, "\n");
}

// dump
void do_print_status(context_t *cx)
{
    int first;
    word_t mcode = STAR(cx->pc);
    if (mcode == OPCODE(1) || mcode == OPCODE(3))
        return;
    fprintf(stderr, "%04X %04X %-12.12s IP:%04X WA:%04X CA:%04X SP:%04X RS:%04X ",
        cx->pc, STAR(cx->pc), opcode_name(STAR(cx->pc)), cx->ip, cx->wa, cx->ca, cx->sp, cx->rs);
    print_stack(cx);
    fprintf(stderr, "\n");
}

void print_s0(context_t *cx)
{
    if (STAR(DEBUG_ADDR) == 0)
        return;
    char *p = &mem[STAR(S0_HEAD)];
    fprintf(stderr, "S0:%04X: [%.64s]\n", STAR(S0_HEAD), p);
}

void print_cstr(context_t *cx, char *title, word_t addr)
{
    char *p = &mem[addr];
    int n = (*p++) & 0x7f;
    if (title && *title)
        fprintf(stderr, "%s:", title);
    fprintf(stderr, "%04X:[%d ", addr, n);
    fprintf(stderr, "%.*s", n, p);
    fprintf(stderr, "]\n");
}

#if 0
void print_dstack(context_t *cx)
{
    word_t w = DSTACK_END - CELLS;
    int first = 1;
    fprintf(stderr, "[");
    for(word_t w = DSTACK_END - CELLS; w >= cx->sp; w -= CELLS) {
        if (first == 0)
            fprintf(stderr, " ");
        first = 0;
        fprintf(stderr, "%04X", STAR(w));
    }
    fprintf(stderr, "]");
}
#endif

void dump_hex(context_t *cx, word_t addr, word_t n)
{
    int eol_flag = 0;
    for (word_t i = 0; i < n; i += 2) {
        eol_flag = 0;
        if (i == 0 || ((addr + i) % 16) == 0)
            fprintf(stderr, "%04x: ", addr + i);
        fprintf(stderr, "%04x ", STAR(addr + i));
        if ((addr + i) % 16 >= 14) {
            fprintf(stderr, "\n");
            eol_flag = 1;
        }
    }
    if (eol_flag == 0)
        fprintf(stderr, "\n");
}

// debugger

void do_debugger(context_t *cx)
{
    int c;
    do_print_status(cx);
    fprintf(stderr, "\n> ");
    while (1) {
        c = getcmd();
        if (c == '\r' || c == '\n') {
            fprintf(stderr, "\n");
            break;
        }
    }
}