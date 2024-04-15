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
    // machine code
    if ((code & 0xf000) != opcode_base) {
undefined:
        fprintf(stderr, "machine_code: undefined instruction: %04X at %04X\n", code, cx->pc);
        return -1;
    }
    switch(code & 0xfff) {
    case 0: // m_nop
        // do nothing
        cx->pc += CELLS;
        break;
    case 1: // m_halt
        //fprintf(stderr, "halt at %04X\n", cx->pc);
        return -1;
    case 2: // m_colon
        do_pushr(cx, cx->ip);
        cx->ip = cx->wa;
        // falling down
    case 3: // m_next
    do_next_label:
        cx->wa = STAR(cx->ip);
        cx->ip += CELLS;
        // falling down
    case 4: // m_run
        cx->ca = STAR(cx->wa);    // WA->CA, seems not @WA->CA
        cx->wa += CELLS;
        cx->pc = cx->ca;
        break;
    case 5: // m_semi
        cx->ip = do_popr(cx);
        cx->pc += CELLS;
        break;
    case 6: // m_jz 
        // conditional bra in thread
        if (do_pop(cx) != 0) {
            cx->ip += CELLS;       // skip branch operand
            cx->pc += CELLS;
            break;
        }
        // falling down
    case 7: // m_jmp 
        // unconditional bra in thread
        addr = STAR(cx->ip);
        cx->ip = addr;
        cx->pc += CELLS;
        break;
    case 8: // m__state
        // interpret/compile state user variable
        do_push(cx, STATE_ADDR);
        cx->pc += CELLS;
        break;
    case 9: // m_create
        do_create(cx);
        cx->pc += CELLS;
        break;
    case 10: // m_key
        // get one byte from keyboard(stdin)
        do_push(cx, fgetc(stdin));
        cx->pc += CELLS;
        break;
    case 11: // m_emit
        // put one byte to screen
        w = do_pop(cx);
        do_emit(cx, w);
        cx->pc += CELLS;
        break;
    case 12: // m_not
        STAR(cx->sp) = !tos(cx);
        cx->pc += CELLS;
        break;
    case 13: // m_xor
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) ^ w;
        cx->pc += CELLS;
        break;
    case 14: // m_or
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) | w;
        cx->pc += CELLS;
        break;
    case 15: // m_and
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) & w;
        cx->pc += CELLS;
        break;
    case 16: // m_h
        do_push(cx, H_ADDR);
        cx->pc += CELLS;
        break;
    case 17: // m_last
        do_push(cx, STAR(LAST_ADDR));
        cx->pc += CELLS;
        break;
    case 18: // m_base
        w = STAR(BASE_ADDR);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 19: // m_type
        n = do_pop(cx);
        addr = do_pop(cx);
        p = &mem[addr];
        while (n-- > 0)
            do_emit(cx, *p++);
        cx->pc += CELLS;
        break;
#if !defined(MINIMUM)
    case 20: // m_space
        do_emit(cx, ' ');
        cx->pc += CELLS;
        break;
    case 21: // m_bl
        do_push(cx, ' ');
        cx->pc += CELLS;
        break;
    case 22: // m_cr
        do_emit(cx, '\r');
        do_emit(cx, '\n');
        cx->pc += CELLS;
        break;
#endif //MINIMUM
    case 23: // m_period
        w = do_pop(cx);
        printf("%d", w);
        cx->pc += CELLS;
        break;
    case 24: // m_div
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) / w;
        cx->pc += CELLS;
        break;
    case 25: // m_mul
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) * w;
        cx->pc += CELLS;
        break;
    case 26: // m_sub
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) - w;
        cx->pc += CELLS;
        break;
    case 27: // m_add
        w = do_pop(cx);
        STAR(cx->sp) = tos(cx) + w;
        cx->pc += CELLS;
        break;
     case 28: // m_rot
        // (n1 n2 n3 -- n2 n3 n1)
        w = STAR(cx->sp + 2 * CELLS);
        STAR(cx->sp + 2 * CELLS) = STAR(cx->sp + CELLS);
        STAR(cx->sp + CELLS) = STAR(cx->sp);
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    case 29: // m_swap
        w = STAR(cx->sp + CELLS);
        STAR(cx->sp + CELLS) = STAR(cx->sp);
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    case 30: // m_drop
        do_pop(cx);
        cx->pc += CELLS;
        break;
    case 31: // m_over
        w = STAR(cx->sp + 2);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 32: // m_dup
        w = tos(cx);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
#if !defined(MINIMUM)
    case 33: // m_comma
        w = tos(cx);
        fprintf(stderr, "m_comma: addr = %04x, value = %04x\n", *wp, w);
        mem[STAR(H_ADDR)] = w;
        mem[H_ADDR] += CELLS;
        cx->pc += CELLS;
        break;
#endif //MINUMUM
    case 34: // m_bytedeposite
        w = do_pop(cx);
        mem[w] = do_pop(cx);
        fprintf(stderr, "mem[%04x] = %04x\n", w, mem[w]);
        cx->pc += CELLS;
        break;
    case 35: // m_exclamation
        w = do_pop(cx);
        STAR(w) = do_pop(cx);
        fprintf(stderr, "mem[%04x] = %04x\n", w, STAR(w));
        cx->pc += CELLS;
        break;
    case 36: // m_bytefetch
        c = mem[tos(cx)];
        STAR(cx->sp) = c;
        cx->pc += CELLS;
        break;
    case 37: // m_atfetch
        w = STAR(tos(cx));
        STAR(cx->sp) = w;
        cx->pc += CELLS;
        break;
    case 38: // m_literal
        w = STAR(cx->ip);
        cx->ip += CELLS;
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 39: // m_execute
        cx->ip = do_pop(cx);
        goto do_next_label;
    case 40: // m_start_colondef
        do_start_colondef(cx);
        cx->pc += CELLS;
        break;
    case 41: // m_end_colondef
        do_end_colondef(cx);
        cx->pc += CELLS;
        break;
    case 42: // m_dictdump
        dump_entry(cx);
        cx->pc += CELLS;
        break;
    case 43: // m_docons
        // constant runtime routine
        w = STAR(cx->wa);
        //fprintf(stderr, "docons: WA:%04X, w = %04x\n", cx->wa, w);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 44: // m_constant
        do_constant(cx);
        cx->pc += CELLS;
        break;
    case 45: // m_find
        do_find(cx);
        cx->pc += CELLS;
        break;
    case 46: // m_compile
        do_compile(cx);
        cx->pc += CELLS;
        break;
    case 47: // m_r2s
        w = do_popr(cx);
        do_push(cx, w);
        cx->pc += CELLS;
        break;
    case 48: // m_s2r
        w = do_pop(cx);
        do_pushr(cx, w);
        cx->pc += CELLS;
        break;
    case 49: // m_gt
        w2 = do_pop(cx);
        w = tos(cx);
        tos(cx) = (w > w2) ? -1 : 0 ;
        cx->pc += CELLS;
        break;
    case 50: // r22s
        w = STAR(cx->rs + CELLS);
        do_push(cx, w);
        cx->pc += CELLS;
        break;       
    case 51: // s2r2
        w = do_pop(cx);
        STAR(cx->rs + CELLS) = w;
        cx->pc += CELLS;
        break;
    case 52: // rswap
        w = STAR(cx->rs);
        STAR(cx->rs) = STAR(cx->rs + CELLS);
        STAR(cx->rs + CELLS) = w;
        cx->pc += CELLS;
        break;
    default:
        goto undefined;
    }
    return 0;   // success result
}
