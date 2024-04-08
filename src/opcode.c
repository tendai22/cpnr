//
//
//

#include <stdio.h>
#include "machine.h"

static int opcode_base = 0xc000;

int machine_code(context_t *cx, word_t code)
{
    word_t addr;
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
    m_next:
        cx->wa = peekMEM(cx, cx->ip);
        cx->ip += 2;
        // falling down
    case 4: // m_run
        cx->ca = peekMEM(cx, cx->wa);
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
        cx->pc += 2;
        addr = peekMEM(cx, cx->pc);
        cx->pc = addr;
        break;
    default:
        goto undefined;
    }
    return 0;   // success result
}
