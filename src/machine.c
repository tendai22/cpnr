//
//
//

#include <stdio.h>
#include "machine.h"

//
// macros
//
#ifdef BIG_ENDIAN
#define lowbyte(v) (((v)/256)&0xff)
#define highbyte(v) ((v)&0xff)
#else
#define highbyte(v) (((v)/256)&0xff)
#define lowbyte(v) ((v)&0xff)
#endif

//
// target ROM/RAM image
//
mem_t mem[65536];


word_t peekMEM(context_t *cx, word_t addr)
{
    return mem[addr];
}

void pokeMEM(context_t *cx, word_t addr, word_t value)
{
    // big endian
#ifdef BIG_ENDIAN
    mem[addr] = highbyte(value);
    mem[addr+1] = lowbyte(value);
#else // little endian
    mem[addr] = lowbyte(value);
    mem[addr+1] = highbyte(value);
#endif
}

void pokeMEM_b(context_t *cx, word_t addr, word_t value)
{
    mem[addr] = value & 0xff;
}

// pushr, popr
void do_pushr(context_t *cx, word_t value)
{
    if (--cx->rs < 0) {
        fprintf(stderr, "rstack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    cx->rstack[cx->rs] = value;
}

word_t do_popr(context_t *cx)
{
    word_t value = cx->rstack[cx->rs];
    if (++cx->rs >= STACK_SIZE) {
        fprintf(stderr, "rstack overflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    return value;
}

word_t do_pop(context_t *cx)
{
    cx->sp++;
    if (cx->sp > 256) {
        fprintf(stderr, "stack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
}

void do_halt(context_t *cx)
{
    cx->halt_flag = 1;
}

void do_machine(context_t *cx)
{
    int i;
    word_t code;
    // one instruction execution loop,
    // check if a break occurs of not
    while (1) {
        // breakpoints?
        for (i = 0; i < BPTBL_SIZE; ++i)
            if (cx->pc == cx->bp[i])
                break;
        if (cx->ss_flag || i < BPTBL_SIZE) {
            // bp occurs
            do_debugger(cx);
        }
        // no break occurs
        // do one instruction
        code = peekMEM(cx, cx->pc);
        if (machine_code(cx, code) != 0)
            break;
        if (cx->halt_flag) {
            fprintf(stderr,"halt:\n");
            break;
        }
    }
}

void do_execute (context_t *cx)
{
    // start inter interpreter
    cx->wa = do_pop(cx);
    // code of m_run(cx);
    cx->ca = peekMEM(cx, cx->wa);
    cx->wa += 2;
    cx->pc = cx->ca;
    // do infinite loop
    do_machine(cx);
}

void do_catch(context_t *cx)
{
    int result;
    result = setjmp(cx->env);    // no disclimination
}

// do_abort
// If the flag is true, types out the last word interpreted, followed by the
// text. Also clears the user's stacks and returns control to the terminal. If
//false, takes no action.
void do_abort(context_t *cx, const char *mes)
{
    char *p;
    int count;
    if (tos(cx)) {
        p = &mem[mem[LAST_ADDR]];      // entry top, word name
        count = *p & 0x1f;
        fprintf(stderr, "%*s %s\n", count, p, mes);
    }
    longjmp(cx->env, tos(cx));
}

int do_mainloop(context_t *cx)
{
    while (1) {
        do_catch(cx);
        if (do_accept(cx) == EOF)
            return -1;
        do_print_s0(cx);
        while (1) {
            do_push(cx, ' ');     // push delimiter
            do_word(cx);
            if (tos(cx) == 0) {
                if (do_accept(cx) == EOF)
                    return -1;
                do_print_s0(cx);
                continue;
            }
            print_cstr(cx, "H", cx->h);
            do_pop(cx);
            do_dup(cx);
            do_find(cx);
            continue;
            if (tos(cx) != 0) {
                do_execute(cx);
            } else {
                do_number(cx);
                do_dup(cx);
                if (tos(cx) != 0) {
                    do_abort(cx, " not found\n");
                }
            }
        }
    }
}

