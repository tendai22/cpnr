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
// word/cell size, per byte
//
#define CELLS 2

//
// taget machine ROM/RAM size
//
#define ROMSTART 0x1000
#define ROMSIZE 0x2000
#define RAMSTART 0x4000
#define RAMSIZE 0x4000

//
// memory map
//
#define DICT_START    0x1000
#define USER_START    0x4000
#define STACK_END     0xff00

// stack area
#define DSTACK_END    STACK_END
#define RSTACK_END    (DSTACK_END-0x100)

// user variables
#define LAST_ADDR     USER_START
#define H_ADDR        (USER_START+2)
#define S0_ADDR       (USER_START+4)
#define STATE_ADDR    (USER_START+6)
#define BASE_ADDR     (USER_START+8)
#define HALT_ADDR     (USER_START+10)
#define COLON_ADDR    (USER_START+12)
#define SEMI_ADDR     (USER_START+14)
#define LITERAL_ADDR  (USER_START+16)
#define DOCONS_ADDR   (USER_START+18)
 
#define MEMSIZE 65536
extern mem_t mem[];

#define STAR(addr) (*((word_t *)&mem[addr]))
#define AMPERSANT(p) ((word_t)((p)-&mem[0]))

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
    int    halt_flag;
    int    ss_flag;
    // input-stream, raw c pointer
    uint8_t *p;
    word_t rest;
    word_t bp[BPTBL_SIZE];      // break point table
} context_t;

// external functions
extern char *str(mem_t *c_ptr);
extern void do_halt(context_t *cx);
extern void do_debugger(context_t *cx);
extern int machine_code(context_t *cx, word_t code);
extern int monitor(context_t *cx);
extern int do_mainloop(context_t *cx);
extern void reset(context_t *cx);

extern int gets_outer(char *buf, int size);
extern void reset_outer(void);
extern void reset_instream(context_t *cx);

// monitor
extern void do_print_status(context_t *cx);
extern void print_s0(context_t *cx);
extern void print_cstr(context_t *cx, char *title, word_t addr);
extern void print_stack(context_t *cx);
extern const char *opcode_name(word_t mcode);
extern void init_optable(void);
extern word_t entry_head(context_t *cx, word_t addr);

// outer interpreter
extern int do_accept(context_t *cx);
extern void do_word(context_t *cx);
extern void do_find(context_t *cx);
extern void do_number(context_t *cx);
extern void do_prompt(context_t *cx);

extern void do_catch(context_t *cx);
extern void do_abort(context_t *cx, const char *message);
extern void do_push(context_t *cx, word_t value);
extern void do_pushr(context_t *cx, word_t value);
extern void do_dup(context_t *cx);
//extern word_t tos(context_t *cx);
extern word_t do_pop(context_t *cx);
extern word_t do_popr(context_t *cx);

extern void do_start_colondef(context_t *cx);
extern void do_end_colondef(context_t *cx);
extern void do_create(context_t *cx);
extern void do_compile_token(context_t *cx);
extern void do_compile_number(context_t *cx);
extern void do_compile(context_t *cx);
extern void do_constant(context_t *cx);
extern void do_emit(context_t *cx, word_t w);
extern void dump_entry(context_t *cx);

#define tos(cx) STAR(cx->sp)

// machine code
extern void m_pushr(context_t *cx, word_t value);
extern void m_next(context_t *cx);
extern word_t m_popr(context_t *cx);
extern void m_run(context_t *cx);
extern void m_jmp_immediate(context_t *cx);
extern void m_jmp(context_t *cx, word_t addr);

#endif //__MACHINE_H
