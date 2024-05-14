//
// opcode.c: machine_code executor
// Apr 14, 2024  Norihiro Kumagai
//

#include <stdio.h>
#include "machine.h"

static int opcode_base = 0xc000;

int machine_code(context_t *cx, word_t code)
{
    word_t addr, *wp, w, w2, n;
    mem_t *p, c;
    int cc, result;
    // machine code
    if ((code & 0xf000) != opcode_base) {
undefined:
        fprintf(stderr, "machine_code: undefined instruction: %04X at %04X\n", code, cx->pc);
        return -1;
    }
    switch(code & 0xfff) {
    //
    // nop, halt
    //
    case 0: // m_nop
        // do nothing
        cx->pc += CELLS;
        break;
    case 1: // m_halt
        //if (STAR(DEBUG_ADDR))
        //    fprintf(stderr, "halt at %04X\n", cx->pc);
        return -1;
    //
    // inner interpreter
    //
    case 2: // m_colon
        do_pushr(cx, cx->ip);
        cx->ip = cx->wa;
        // falling down
    case 3: // m_next
    do_next_label:
        cx->wa = STAR(cx->ip);
        if (STAR(DEBUG_ADDR))
            print_next(cx, cx->wa);
        cx->ip += CELLS;
        // falling down
    case 4: // m_run
    do_run_label:
        cx->ca = STAR(cx->wa);    // WA->CA, seems not @WA->CA
        cx->wa += CELLS;
        cx->pc = cx->ca;
        break;
    case 5: // m_startdoes
        do_push(cx, cx->wa);
        cx->wa = cx->pc + CELLS;
        goto do_run_label;
    case 6: // m_semi
        cx->ip = do_popr(cx);
        cx->pc += CELLS;
        break;
    case 7: // m_execute
        cx->ip = do_pop(cx);
        goto do_next_label;

    case 16: // m_literal
        w = STAR(cx->ip);
        cx->ip += CELLS;
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 17: // m_qbranch 
        // conditional branch in thread
        if (do_pop(cx) != 0) {
            cx->ip += CELLS;       // skip branch operand
            cx->pc += CELLS;
            break;
        }
        // falling down
    case 18: // m_branch 
        // unconditional bra in thread
        addr = STAR(cx->ip);
        cx->ip = addr;
        cx->pc += CELLS;
        break;
    //
    // fetch and deposite (memory read/write)
    //
    case 24: // m_bytedeposite
        w = do_pop(cx);
        mem[w] = do_pop(cx);
        if (STAR(DEBUG_ADDR))
            fprintf(stderr, "mem[%04x] = %04x\n", w, mem[w]);
        cx->pc += CELLS;
        break;
    case 25: // m_exclamation
        w = do_pop(cx);
        STAR(w) = do_pop(cx);
        if (STAR(DEBUG_ADDR))
            fprintf(stderr, "mem[%04x] = %04x\n", w, STAR(w));
        cx->pc += CELLS;
        break;
    case 26: // m_bytefetch
        c = mem[tos(cx)];
        STAR(cx->sp) = c;
        cx->pc += CELLS;
        break;
    case 27: // m_atfetch
        w = STAR(tos(cx));
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    //
    // stack operations
    //
    case 32: // m_rot
        // (n1 n2 n3 -- n2 n3 n1)
        w = STAR(cx->sp + 2 * CELLS);
        STAR(cx->sp + 2 * CELLS) = STAR(cx->sp + CELLS);
        STAR(cx->sp + CELLS) = STAR(cx->sp);
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    case 33: // m_swap
        w = STAR(cx->sp + CELLS);
        STAR(cx->sp + CELLS) = STAR(cx->sp);
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    case 34: // m_drop
        do_pop(cx);
        cx->pc += CELLS;
        break;
    case 35: // m_over
        w = STAR(cx->sp + 2);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 36: // m_dup
        w = tos(cx);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    //
    // return stack operations
    //
    case 37:    // m_rsp
        do_push(cx, cx->rs);
        cx->pc += CELLS;
        break;
    case 38:    // m_r2s
        w = do_popr(cx);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 39: // m_s2r
        w = do_pop(cx);
        do_pushr(cx, w);
        cx->pc += CELLS;
        break;
    case 40: // m_r22s
        w = STAR(cx->rs + CELLS);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 41: // m_s2r2
        w = do_pop(cx);
        STAR(cx->rs + CELLS) = w;
        cx->pc += CELLS;
        break;
    //
    // operators
    //
    case 48: // m_not
        STAR(cx->sp) = tos(cx) ? 0 : -1;
        cx->pc += CELLS;
        break;
    case 49: // m_xor
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) ^ w;
        cx->pc += CELLS;
        break;
    case 50: // m_or
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) | w;
        cx->pc += CELLS;
        break;
    case 51: // m_and
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) & w;
        cx->pc += CELLS;
        break;
    case 52: // m_div
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) / w;
        cx->pc += CELLS;
        break;
    case 53: // m_mul
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) * w;
        cx->pc += CELLS;
        break;
    case 54: // m_sub
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) - w;
        cx->pc += CELLS;
        break;
    case 55: // m_add
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) + w;
        cx->pc += CELLS;
        break;
    case 56: // m_gt
        w2 = do_pop(cx);
        w = tos(cx);
        tos(cx) = ((sword_t)w > (sword_t)w2) ? -1 : 0 ;
        cx->pc += CELLS;
        break;
    case 57: // m_slashmod
        // fprintf(stderr, "%04x %d %d /mod ============================\n", cx->ip, w2, w);
        w = do_pop(cx);     // n2
        w2 = do_pop(cx);    // n1
        do_push(cx, w2 % w);
        do_push(cx, w2 / w);
        cx->pc += CELLS;
        break;
    //
    // i/o primitives
    //
    case 64: // m_getch
        //
        do_getch(cx);
        cx->pc += CELLS;
        break;
    case 65: // m_emit
        // put one byte to screen
        w = do_pop(cx);
        do_emit(cx, w);
        cx->pc += CELLS;
        break;
    case 66: // m_space
        do_emit(cx, ' ');
        cx->pc += CELLS;
        break;
    case 67: // m_cr
        do_emit(cx, '\r');
        do_emit(cx, '\n');
        cx->pc += CELLS;
        break;
    case 68: // m_period
        w = do_pop(cx);
        printf("%d", w); fflush(stdout);
        cx->pc += CELLS;
        break;
    case 69: // m_type
        n = do_pop(cx);
        addr = do_pop(cx);
        p = &mem[addr];
        while (n-- > 0)
            do_emit(cx, *p++);
        cx->pc += CELLS;
        break;
    case 70: // m_h
        do_push(cx, H_ADDR);
        cx->pc += CELLS;
        break;
    case 71: // m_last
        do_push(cx, STAR(LAST_ADDR));
        cx->pc += CELLS;
        break;
    case 72: // m_base
        w = STAR(BASE_ADDR);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
#if !defined(MINIMUM)
    case 73: // m_bl
        do_push(cx, ' ');
        cx->pc += CELLS;
        break;
#endif //MINIMUM
    case 74: // m_comma
        w = tos(cx);
        if (STAR(DEBUG_ADDR))
            fprintf(stderr, "m_comma: addr = %04x, value = %04x\n", *wp, w);
        STAR(STAR(H_ADDR)) = w;
        STAR(H_ADDR) += CELLS;
        cx->pc += CELLS;
        break;
    //
    // high level words
    //
    case 75: // m_create
        do_create(cx);
        cx->pc += CELLS;
        break;
    case 77: // m_start_colondef
        do_start_colondef(cx);
        cx->pc += CELLS;
        break;
    case 78: // m_end_colondef
        do_end_colondef(cx);
        cx->pc += CELLS;
        break;
    case 79: // m_dictdump
        dump_entry(cx);
        cx->pc += CELLS;
        break;
#if 0 //!defined(MINIMUM)
    case 80: // m_docons
        // constant runtime routine
        w = STAR(cx->wa);
        //fprintf(stderr, "docons: WA:%04X, w = %04x\n", cx->wa, w);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 81: // m_constant
        do_constant(cx);
        cx->pc += CELLS;
        break;
#endif //!MINIMUM
    case 82: // m_start_compile
        STAR(STATE_ADDR) = 1;
        cx->pc += CELLS;
        break;
    case 83: // m_end_compile
        STAR(STATE_ADDR) = 0;
        cx->pc += CELLS;
        break;
    case 84: // m_add_rsp
        cx->rs += do_pop(cx);
        cx->pc += CELLS;
        break;
    case 85: // m_find
        do_find(cx);
        cx->pc += CELLS;
        break;
    case 86: // m_compile
        do_compile(cx);
        cx->pc += CELLS;
        break;
    case 87: // m_word
        do_word(cx);
        cx->pc += CELLS;
        break;
    case 88: // m__state
        // interpret/compile state user variable
        do_push(cx, STATE_ADDR);
        cx->pc += CELLS;
        break;
    case 89: // m_sp
        do_push(cx, cx->sp);
        cx->pc += CELLS;
        break;
    case 98: // m_getline
        // ( n addr -- -1 (ready)|0 (eof) )
        w = do_pop(cx);
        n = do_pop(cx);
        result = do_getline(cx, &mem[w], n);
        result = (result == 0) ? -1 : 0;
        do_push(cx, result);
        cx->pc += CELLS;
        break;
#if 0
    case 99: // m_accept
        // getc from stdin
        fprintf(stderr, "enter accept\n");
        n = do_accept(cx);
        fprintf(stderr, "do_accept: n = %d\n", n);
        if (n == EOF) {
            // empty buffer
            p = &mem[STAR(S0_ADDR)];
            *p = 0;
        }
        print_s0(cx);
        cx->pc += CELLS;
        break;
#endif
    default:
        goto undefined;
    }
    return 0;   // success result
}
