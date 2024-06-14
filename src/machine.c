//
//
//

#include <stdio.h>
#include "machine.h"
#include "user.h"

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

int do_machine(context_t *cx)
{
    int i, result;
    word_t code;
    // one instruction execution loop,
    // check if a break occurs of not
    //STAR(DEBUG_ADDR) = 1;
    cx->halt_flag = 0;
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
        //fprintf(stderr, "pc: %04x code: %04x\n", cx->pc, code);
        if (STAR(DEBUG_ADDR) & 1)
            do_print_status(cx);
        if ((result = machine_code(cx, code)) != 0) {
            if (result < 0) {
                fprintf(stderr, "trap: result = %d, lnum = %d\n", result, lnum);
                //longjmp(cx->env, -result);
            }
            return result;
        }
    }
}

void do_execute (context_t *cx)
{
    word_t w;
    int flag = 1;
    char *p;
    // start inter interpreter
    cx->wa = w = do_pop(cx);
    // code of m_run(cx);
    cx->ca = STAR(cx->wa);
    if (STAR(DEBUG_ADDR)&4) {
        if (flag) {
            if (w == STAR(COLON_HEAD))
                p = "\005COLON"; 
            else if (w == STAR(SEMI_HEAD))
                p = "\004SEMI";
            else if (w == STAR(LITERAL_HEAD))
                p = "\005dolit";
            else {
                p = &mem[entry_head(cx, w)];
            }
            fprintf(stderr, "X:%04x (%.*s)\n", w, (*p)&0x1f, p+1);
        } else {
            fprintf(stderr, "X:%04x (%d)\n", w, w);
        }
    }
    cx->wa += CELLS;
    cx->pc = cx->ca;
    cx->ip = HALT_HEAD;     // ipはxtの置き場を指すようにする。
                            // それはHALT_ADDRだ。
    // do infinite loop
    do_machine(cx);
}

#if 0
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
#endif

// do_abort
// If the flag is true, types out the last word interpreted, followed by the
// text. Also clears the user's stacks and returns control to the terminal. If
//false, takes no action.
void do_abort(context_t *cx, int errno, const char *mes)
{
    const mem_t *p = &mem[STAR(DP_ADDR)];
    if (errno >= 0) {
        fprintf(stderr, ">>> %d: %d %.*s %s\n", (int)lnum, errno, (*p)&0x1f, p+1, mes);
    } else if (errno == -1) { // bye
        fprintf(stderr, "bye\n");
    }
    longjmp(cx->env, errno);
    //fprintf(stderr, "after longjmp\n");
}

void print_emit(context_t *cx, char c)
{
    fputc(c, stderr);
    fflush(stderr);
}

void print_crlf(context_t *cx)
{
    print_emit(cx, '\n');
}

int do_mainloop(context_t *cx)
{
    word_t flag, caddr, cross;
    int count, result;
    char *p;
    while (1) {
        // do_catch
        //do_catch(cx);
        if ((result = setjmp(cx->env)) != 0) {
            fprintf(stderr, "abort result = %d\n", result);
            if (result < 0)
                return -1;
            // reset input stream
            reset_instream(cx);
        }
        cx->sp = DSTACK_END;
        cx->rs = RSTACK_END;
        count = 0;
        do_prompt(cx);
        if (do_accept(cx) == EOF)
            return -1;
        print_s0(cx);
        while (1) {
            //print_stack(cx);
            do_push(cx, ' ');     // push delimiter
            do_word(cx);    // (delim -- addr)
            if (tos(cx) == 0) {
                do_pop(cx); // discard it
                do_prompt(cx);
                if (do_accept(cx) == EOF)
                    return -1;
                print_s0(cx);
                continue;
            }
            caddr = tos(cx);    // save c-addr for re-find
            p = &mem[caddr];
            //print_cstr(cx, "H", STAR(DP_ADDR));
            //print_stack(cx); print_crlf(cx);
            cross = STAR(CROSS_ADDR);
            do_find(cx, STAR(LAST_ADDR));
            if (tos(cx) == 0) {
                //fprintf(stderr, "cross find: \"%.*s\"\n", *p&0x1f, p+1);
                tos(cx) = caddr;
                do_find(cx, cross);
            }
            //if (do_pop(cx))
            //    do_pop(cx); // clear the result of do_find
            //continue;
            //print_emit(cx, 'A'); 
            //print_stack(cx);print_crlf(cx);
            if ((flag = do_pop(cx)) != 0) {
                if (STAR(STATE_ADDR) && (flag & 0x8000) == 0) {
                    // re scan target dict
                    tos(cx) = caddr;
                    do_find(cx, STAR(LAST_ADDR));
                    if ((flag = do_pop(cx)) == 0) {
                        fprintf(stderr, "do_mainloop: line %d: \"%.*s\" not found unexlectedly in compiling\n", lnum, *p&0x1f, p+1);
                        return -1;
                    }
                    do_compile_token(cx);
                } else {
                    if (STAR(CROSS_ADDR)) {    // if executing, re scan on host dictionary
                        // search host dictionary and execute it
                        tos(cx) = caddr;
                        do_find(cx, STAR(CROSS_ADDR));
                        if ((flag = do_pop(cx)) == 0) {
                            // unexpected not-fund in host dictionary
                            fprintf(stderr, "do_mainloop: line %d: \"%.*s\" not found in host dictionary\n", lnum, *p&0x1f, p+1);
                            return -1;
                        }
                        //fprintf(stderr,"immediate: exec \"%.*s\"\n", *p&0x1f, p+1);
                    }
                    do_execute(cx);
                }
            } else {
                do_push(cx, STAR(DP_ADDR));
                do_number(cx);  // (addr -- n r)
                if (do_pop(cx) != 0) {
                    do_abort(cx, 1, "not found");
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

