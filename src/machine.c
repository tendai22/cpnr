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
    cx->rs -= 2;
    if (cx->rs < 0) {
        fprintf(stderr, "rstack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    word_mem(cx->rs) = value;
}

word_t do_popr(context_t *cx)
{
    word_t value = word_mem(cx->rs);
    cx->rs += 2;
    if (cx->rs > RSTACK_END) {
        fprintf(stderr, "rstack overflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    return value;
}

word_t do_pop(context_t *cx)
{
    word_t value = word_mem(cx->sp);
    cx->sp += 2;
    if (cx->sp > DSTACK_END) {
        fprintf(stderr, "stack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    return value;
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
        code = word_mem(cx->pc);
        do_print_status(cx);
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
    cx->ca = word_mem(cx->wa);
    cx->wa += 2;
    cx->pc = cx->ca;
    cx->ip = HALT_ADDR;     // ipはxtの置き場を指すようにする。
                            // それはHALT_ADDRだ。
    fprintf(stderr, "execute: WA: %04x CA:%04x\n", cx->wa, cx->ca);
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
    word_t flag;
    while (1) {
        do_catch(cx);
        if (do_accept(cx) == EOF)
            return -1;
        do_print_s0(cx);
        while (1) {
            print_stack(cx);
            do_push(cx, ' ');     // push delimiter
            do_word(cx);    // (delim -- addr)
            if (tos(cx) == 0) {
                do_pop(cx); // discard it
                if (do_accept(cx) == EOF)
                    return -1;
                do_print_s0(cx);
                continue;
            }
            print_cstr(cx, "H", word_mem(H_ADDR));
            print_stack(cx);
            do_find(cx);
            //if (do_pop(cx))
            //    do_pop(cx); // clear the result of do_find
            //continue;
            if ((flag = do_pop(cx)) != 0) {
                if (word_mem(STATE_ADDR) && (flag & 0x8000) == 0)
                    do_compile_token(cx);
                else
                    do_execute(cx);
            } else {
                do_push(cx, word_mem(H_ADDR));
                do_number(cx);  // (addr -- n r)
                if (do_pop(cx) != 0) {
                    do_abort(cx, " not found\n");
                }
                if (word_mem(STATE_ADDR))
                    do_compile_number(cx);
                // push value, do nothing
            }
        }
    }
}

