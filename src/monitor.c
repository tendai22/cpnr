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
    tcgetattr(0, &tb);
    saved_flag = 1;
    cfmakeraw(&tb);
    tcsetattr(0, TCSANOW, &tb);
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

void monitor (context_t *cx)
{
    int c, result;
    while (1) {
        // monitor prompt
        fprintf(stderr, "]");
        fflush(stderr);
        c = getcmd();
        if (c == GO_CMD) {
            do_mainloop(cx);
        } else if (c == 'r') {
            reset(cx);
        } else {
            // not found
            fprintf(stderr, "monitor: bad command %02x\n", c);
        }
    }
}

// debugger

void debugger(context_t *cx)
{
    int c;
    fprintf(stderr, "%04x IP:%04x WA:%04x CA:%04x AH:%04x AL:%04x ",
        cx->pc, cx->ip, cx->wa, cx->ca, cx->ah, cx->al);
    fprintf(stderr, "SP:");
    for (int i = 254; i >= cx->sp; i -= 2) {
        fprintf(stderr, " %04X", peekMEM(cx, i));
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "RS: ");
    for (int i = 254; i >= cx->rs; i -= 2) {
        fprintf(stderr, " %04X", peekMEM(cx, i));
    }
    fprintf(stderr, "\n> ");
    while (1) {
        c = getcmd();
        if (c == '\r' || c == '\n') {
            fprintf(stderr, "\n");
            break;
        }
    }
}