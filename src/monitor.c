//
// monitor.c:  cpu emulator execution and debugger
//

#include <stdio.h>
#include <setjmp.h>
#include <termios.h>
#include <unistd.h>
#include "machine.h"


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
    return c;
}

int monitor (context_t *cx)
{
    int c, result;
    while (1) {
        // monitor prompt
        fprintf(stderr, "]");
        fflush(stderr);
        c = getcmd();
        if (c < 0)
            return -1;
        if (c == GO_CMD) {
            if (do_mainloop(cx) < 0)
                return -1;
        } else if (c == 'r') {
            reset(cx);
        } else {
            // not found
            fprintf(stderr, "monitor: bad command %02x\n", c);
        }
    }
}

// dump
void do_print_status(context_t *cx)
{
    fprintf(stderr, "%04x IP:%04x WA:%04x CA:%04x AH:%04x AL:%04x SP:%04x RS:%04x",
        cx->pc, cx->ip, cx->wa, cx->ca, cx->ah, cx->al, cx->sp, cx->rs);
    fprintf(stderr, "\n");
    fprintf(stderr, "SP: ");
    for (int i = 254; i >= cx->sp; i -= 2) {
        fprintf(stderr, " %04X", cx->stack[i]);
    }
    fprintf(stderr, "  RS: ");
    for (int i = 254; i >= cx->rs; i -= 2) {
        fprintf(stderr, " %04X", cx->rstack[i]);
    }
    fprintf(stderr, "\n");
}

void do_print_s0(context_t *cx)
{
    char *p = MEMptr(cx, cx->s0);
    fprintf(stderr, "S0:%04X: ", cx->s0);
    for (int i = 0; i < 16; ++i)
        fprintf(stderr, "%02X ", *p++);
    fprintf(stderr, "\n");
}

void do_print_here(context_t *cx)
{
    char *p = MEMptr(cx, cx->h);
    fprintf(stderr, "H:%04X: ", cx->h);
    for (int i = 0; i < 16; ++i)
        fprintf(stderr, "%02X ", *p++);
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