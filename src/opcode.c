//
//
//

#include <stdio.h>
#include "machine.h"

static int opcode_base = 0xc000;

int machine_code(context_t *cx, word_t code)
{
    word_t addr, *wp, w, n;
    char *p, c;
    // machine code
    if ((code & 0xf000) != opcode_base) {
undefined:
        fprintf(stderr, "machine_code: undefined instruction: %04X at %04X\n", code, cx->pc);
        return -1;
    }
    switch(code & 0xfff) {
    case 0: // m_nop
        // do nothing
        cx->pc += 2;
        break;
    case 1: // m_halt
        fprintf(stderr, "halt at %04X\n", cx->pc);
        return -1;
    case 2: // m_colon
        do_pushr(cx, cx->ip);
        cx->ip = cx->wa;
        // falling down
    case 3: // m_next
    do_next_label:
        cx->wa = word_mem(cx->ip);
        cx->ip += 2;
        // falling down
    case 4: // m_run
        cx->ca = word_mem(cx->wa);
        cx->wa += 2;
        cx->pc = cx->ca;
        break;
    case 5: // m_semi
        cx->ip = do_popr(cx);
        cx->pc += 2;
        break;
    case 6: // m_jnz 
        // conditional bra in thread
        if (tos(cx) == 0) {
            cx->pc += 2;       // skip branch operand
            break;
        }
        // falling down
    case 7: // m_jmp 
        // unconditional bra in thread
        addr = word_mem(cx->pc + 2);
        cx->pc = addr;
        break;
    case 8: // m_state
        // interpret/compile state user variable
        do_push(cx, cx->state);
        cx->pc += 2;
        break;
    case 9: // m_create
        do_create(cx);
        cx->pc += 2;
        break;
    case 10: // m_key
        // get one byte from keyboard(stdin)
        do_push(cx, fgetc(stdin));
        cx->pc += 2;
        break;
    case 11: // m_emit
        // put one byte to screen
        w = do_pop(cx);
        do_emit(cx, w);
        cx->pc += 2;
        break;
    case 12: // m_not
        word_mem(cx->sp) = !tos(cx);
        cx->pc += 2;
        break;
    case 13: // m_xor
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) ^ w;
        cx->pc += 2;
        break;
    case 14: // m_or
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) | w;
        cx->pc += 2;
        break;
    case 15: // m_and
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) & w;
        cx->pc += 2;
        break;
    case 16: // m_h
        do_push(cx, cx->h);
        cx->pc += 2;
        break;
    case 17: // m_last
        do_push(cx, cx->last);
        cx->pc += 2;
        break;
    case 18: // m_base
        w = peekMEM(cx, cx->base);
        do_push(cx, w);
        cx->pc += 2;
        break;
    case 19: // m_type
        n = do_pop(cx);
        addr = do_pop(cx);
        p = &mem[addr];
        while (n-- > 0)
            do_emit(cx, *p++);
        cx->pc += 2;
        break;
    case 20: // m_space
        do_emit(cx, ' ');
        cx->pc += 2;
        break;
    case 21: // m_bl
        do_push(cx, ' ');
        cx->pc += 2;
        break;
    case 22: // m_cr
        do_emit(cx, '\r');
        do_emit(cx, '\n');
        cx->pc += 2;
        break;
    case 23: // m_period
        w = do_pop(cx);
        printf("%d", w);
        cx->pc += 2;
        break;
    case 24: // m_div
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) / w;
        cx->pc += 2;
        break;
    case 25: // m_mul
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) * w;
        cx->pc += 2;
        break;
    case 26: // m_sub
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) - w;
        cx->pc += 2;
        break;
    case 27: // m_add
        w = do_pop(cx);
        word_mem(cx->sp) = tos(cx) + w;
        cx->pc += 2;
        break;
     case 28: // m_rot
        w = word_mem(cx->sp - 4);
        word_mem(cx->sp - 4) = word_mem(cx->sp - 2);
        word_mem(cx->sp - 2) = word_mem(cx->sp);
        word_mem(cx->sp) = w;
        cx->pc += 2;
        break;
    case 29: // m_swap
        w = word_mem(cx->sp - 2);
        word_mem(cx->sp - 2) = tos(cx);
        word_mem(cx->sp) = w;
        cx->pc += 2;
        break;
    case 30: // m_drop
        do_pop(cx);
        cx->pc += 2;
        break;
    case 31: // m_over
        w = word_mem(cx->sp - 2);
        do_push(cx, w);
        cx->pc += 2;
        break;
    case 32: // m_dup
        c = tos(cx);
        do_push(cx, c);
        cx->pc += 2;
        break;
    case 33: // m_comma
        w = tos(cx);
        fprintf(stderr, "m_comma: addr = %04x, value = %04x\n", *wp, w);
        mem[mem[H_ADDR]] = w;
        mem[H_ADDR] += 2;
        cx->pc += 2;
        break;
    case 34: // m_bytedeposite
        w = do_pop(cx);
        mem[w] = do_pop(cx);
        fprintf(stderr, "mem[%04x] = %04x\n", w, mem[w]);
        cx->pc += 2;
        break;
    case 35: // m_exclamation
        w = do_pop(cx);
        word_mem(w) = do_pop(cx);
        fprintf(stderr, "mem[%04x] = %04x\n", w, word_mem(w))
        cx->pc += 2;
        break;
    case 36: // m_bytefetch
        c = mem[do_pop(cx)];
        word_mem(cx->sp) = c;
        cx->pc += 2;
        break;
    case 37: // m_atfetch
        w = word_mem(do_pop(cx));
        word_mem(cx->sp) = w;
        cx->pc += 2;
        break;
    case 38: // m_literal
        cx->pc += 2;
        w = word_mem(cx->pc);
        do_push(cx, w);
        cx->pc += 2;
        break;
    case 39: // m_execute
        cx->ip = do_pop(cx);
        goto do_next_label;
    default:
        goto undefined;
    }
    return 0;   // success result
}
