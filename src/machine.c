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
        code = STAR(cx->pc);
        if (STAR(DEBUG_ADDR) & 1)
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
    cx->ca = STAR(cx->wa);
    cx->wa += CELLS;
    cx->pc = cx->ca;
    cx->ip = HALT_ADDR;     // ipはxtの置き場を指すようにする。
                            // それはHALT_ADDRだ。
    //fprintf(stderr, "execute: WA: %04x CA:%04x\n", cx->wa, cx->ca);
    // do infinite loop
    do_machine(cx);
}

void do_catch(context_t *cx)
{
    int result;
    result = setjmp(cx->env);    // no disclimination
    //fprintf(stderr, "setjmp: result = %d\n", result);
    if (result != 0) {
        // rewind stack
        // reset input stream
        reset_instream(cx);
        cx->sp = DSTACK_END;
        cx->rs = RSTACK_END;
    }
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
        p = &mem[STAR(H_ADDR)];      // entry top, word name
        count = *p & 0x1f;
        fprintf(stderr, "%.*s %s\n", count, p+1, mes);
    }
    longjmp(cx->env, tos(cx));
    fprintf(stderr, "after longjmp\n");
}

int do_mainloop(context_t *cx)
{
    word_t flag;
    int count, result;
    while (1) {
        // do_catch
        do_catch(cx);
        if ((result = setjmp(cx->env)) != 0) {
            // reset input stream
            reset_instream(cx);
            cx->sp = DSTACK_END;
            cx->rs = RSTACK_END;
        }
        count = 0;
        if (do_accept(cx) == EOF)
            return -1;
        print_s0(cx);
        while (1) {
            //print_stack(cx);
            do_push(cx, ' ');     // push delimiter
            do_word(cx);    // (delim -- addr)
            if (tos(cx) == 0) {
                do_pop(cx); // discard it
                if (do_accept(cx) == EOF)
                    return -1;
                print_s0(cx);
                continue;
            }
            //print_cstr(cx, "H", STAR(H_ADDR));
            //print_stack(cx);
            do_find(cx);
            //if (do_pop(cx))
            //    do_pop(cx); // clear the result of do_find
            //continue;
            if ((flag = do_pop(cx)) != 0) {
                if (STAR(STATE_ADDR) && (flag & 0x8000) == 0)
                    do_compile_token(cx);
                else
                    do_execute(cx);
            } else {
                do_push(cx, STAR(H_ADDR));
                do_number(cx);  // (addr -- n r)
                if (do_pop(cx) != 0) {
                    do_push(cx, 1);
                    do_abort(cx, "not found\n");
                    //fprintf(stderr, "b count = %d\n", count++);
                    //break;
                }
                if (STAR(STATE_ADDR)) {
                    do_compile_number(cx);
                    // push value, do nothing
                }
            }
        }
    }
}

