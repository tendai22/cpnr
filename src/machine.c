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

char *MEMptr(context_t *cx, word_t addr) {
    return &mem[addr];
}

// pushr, popr
void do_pushr(context_t *cx, word_t value)
{
    if (--cx->rs < 0) {
        fprintf(stderr, "rstack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt();
    }
    cx->rstack[cx->rs] = value;
}

word_t do_popr(context_t *cx)
{
    word_t value = cx->rstack[cx->rs];
    if (++cx->rs >= STACK_SIZE) {
        fprintf(stderr, "rstack overflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt();
    }
    return value;
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
    }
}

void do_execute (context_t *cx)
{
    // start inter interpreter
    cx->wa = do_pop(cx);
    m_run(cx);
    // do infinite loop
    do_machine(cx);
}



void do_mainloop(context_t *cx)
{
    while (1) {
        do_catch(cx);
        do_accept(cx);
        while (1) {
            do_push(cx, ' ');     // push delimiter
            do_word(cx);
            if (tos(cx) == 0) {
                continue;
            }
            continue;   // word debug
            do_dup(cx);
            do_find(cx);
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

int machine_code(context_t *cx, word_t code)
{
    word_t addr;
    // machine code
    if ((code & 0xc000) != 0xc000) {
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
