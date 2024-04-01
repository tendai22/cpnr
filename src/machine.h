//
//
//

#if !defined(__MACHINE_H)
#include <inttypes.h>
#include <setjmp.h>

typedef uint16_t word_t;
typedef uint8_t mem_t;    // ROM/RAM memory array for the target machine

#define MAX_VARIABLE 20

//
// little or big
//
#define BIG_ENDIAN 1

//
// taget machine ROM/RAM size
//
#define ROMSTART 0x1000
#define ROMSIZE 0x2000
#define RAMSTART 0x4000
#define RAMSIZE 0x4000

extern mem_t rom[];
extern mem_t ram[];

#define STACK_SIZE 256
#define BPTBL_SIZE 16

typedef struct _ctx {
    jmp_buf env;
    word_t ip;      // instruction pointer
    word_t wa;      // working address
    word_t ca;      // code address
    word_t rs;      // return stack pointer
    word_t sp;      // data stack pointer
    word_t ah;
    word_t al;      // accumulator high, low
    word_t pc;
    int    ss_flag;
    word_t var[MAX_VARIABLE]; // user variable array
    word_t stack[STACK_SIZE];
    word_t rstack[STACK_SIZE];
    word_t bp[BPTBL_SIZE];
} context_t;

#define HERE 1
#define BASE 2
#define STATE 3
#define LAST 4

// machine memory
extern word_t peekMEM(context_t *cx, word_t addr);
extern void pokeMEM(context_t *cx, word_t addr, word_t value);
extern void pokeMEM_b(context_t *cx, word_t addr, word_t value);
extern char *MEMptr(context_t *cx, word_t addr);


// esternal functions
extern char *str(mem_t *c_ptr);
extern void do_halt(void);
extern void do_debugger(context_t *cx);
extern void machine_code(context_t *cx, word_t code);
extern void monitor(context_t *cx);
extern void do_mainloop(context_t *cx);
extern void reset(context_t *cx);

extern void do_catch(context_t *cx);
extern void do_abort(context_t *cx, const char *message);
extern void do_push(context_t *cx, word_t value);
extern void do_pushr(context_t *cx, word_t value);
extern void do_dup(context_t *cx);
extern void do_halt(void);
extern word_t tos(context_t *cx);
extern word_t do_pop(context_t *cx);
extern word_t do_popr(context_t *cx);


// machine code
extern void m_pushr(context_t *cx, word_t value);
extern void m_next(context_t *cx);
extern word_t m_popr(context_t *cx);
extern void m_run(context_t *cx);
extern void m_jmp_immediate(context_t *cx);
extern void m_jmp(context_t *cx, word_t addr);

#endif //__MACHINE_H
