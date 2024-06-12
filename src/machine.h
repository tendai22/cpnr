//
// narrowForth C Portable machine definitions
//  Norihiro Kumagai 2024-5-26

#if !defined(__MACHINE_H)
#include <inttypes.h>
#include <setjmp.h>

typedef uint16_t word_t;
typedef int16_t sword_t;
typedef uint8_t mem_t;    // ROM/RAM memory array for the target machine

#define MAX_VARIABLE 20

//
// little or big
//
//#define BIG_ENDIAN 1      // in <stdlib.h> we have it

//
// word/cell size, per byte
//
#define CELLS 2

//
// taget machine ROM/RAM size
//

//
// memory map
//
#define USER_PAGE     0xf000
#define DSTACK_END    (USER_PAGE+256)
#define TIB_START     DSTACK_END
#define RSTACK_END    (TIB_START+256)
#define USER_START    (RSTACK_END)
#define USER_END      (RSTACK_END+256)

// user variables
// are moved to user.h

#define MEMSIZE 65536
extern mem_t mem[];

#define STAR(addr) (*((word_t *)&mem[addr]))
#define AMPERSANT(p) ((word_t)((p)-&mem[0]))

//
// break point address table
//
#define BPTBL_SIZE 16

typedef struct _ctx {
    jmp_buf env;
    word_t ip;      // instruction pointer
    word_t wa;      // working address
    word_t ca;      // code address
    word_t rs;      // return stack pointer
    word_t sp;      // data stack pointer
    word_t pc;
    int    halt_flag;
    int    ss_flag;
    // input-stream, raw c pointer
    uint8_t *p;
    word_t rest;
    word_t bp[BPTBL_SIZE];      // break point table
} context_t;

// C virtual cpu emulation
#define OPCODE_BASE 0x7000
#define OPCODE(n) (OPCODE_BASE+((n)&0xff))

// external functions
extern char *str(mem_t *c_ptr);
extern void do_halt(context_t *cx);
extern void do_debugger(context_t *cx);
extern int machine_code(context_t *cx, word_t code);
extern int monitor(context_t *cx);
extern int do_mainloop(context_t *cx);
extern void reset(context_t *cx);

extern int getch_outer(void);
extern void reset_outer(void);
extern void reset_instream(context_t *cx);
extern int get_instream(context_t *cx);

extern void do_savefile(const char *path, word_t start, word_t end, word_t last);
extern void do_execute(context_t *cx);

// monitor
extern void do_print_status(context_t *cx);
extern void print_s0(context_t *cx);
extern void print_cstr(context_t *cx, char *title, word_t addr);
extern void print_stack(context_t *cx);
extern const char *opcode_name(word_t mcode);
extern void init_optable(void);
extern word_t entry_head(context_t *cx, word_t addr);
extern void print_next(context_t *cx, word_t xt);

// outer interpreter
extern const char *output_file;
extern int do_accept(context_t *cx);
extern int do_getline(context_t *cx, char *buf, int size);
extern void do_getch(context_t *cx);
extern void do_word(context_t *cx);
extern void do_find(context_t *cx, word_t start);
extern void do_number(context_t *cx);
extern void do_prompt(context_t *cx);
extern int gets_outer(mem_t *buf, int len);
extern int getch(context_t *cx);

extern void do_catch(context_t *cx);
extern void do_abort(context_t *cx, int errno, const char *message);
extern int lnum;
extern int outer_flag;
extern void do_lnum(context_t *cx);
extern void do_push(context_t *cx, word_t value);
extern void do_pushr(context_t *cx, word_t value);
extern void do_dup(context_t *cx);
//extern word_t tos(context_t *cx);
extern word_t do_pop(context_t *cx);
extern word_t do_popr(context_t *cx);

extern int  do_quote(context_t *cx);
extern void do_colondef(context_t *cx);
extern void do_semidef(context_t *cx);
extern void do_create(context_t *cx);
extern void do_compile_token(context_t *cx);
extern void do_compile_number(context_t *cx);
//extern void do_compile(context_t *cx);
//extern void do_constant(context_t *cx);
extern void do_emit(context_t *cx, word_t w);
extern void dump_entry(context_t *cx);
extern word_t code_addr(word_t entry);

#define tos(cx) STAR(cx->sp)

// machine code
extern void m_pushr(context_t *cx, word_t value);
extern void m_next(context_t *cx);
extern word_t m_popr(context_t *cx);
extern void m_run(context_t *cx);
extern void m_jmp_immediate(context_t *cx);
extern void m_jmp(context_t *cx, word_t addr);

// double length integer
extern uint32_t do_dpop(context_t *cx);
extern void do_dpush(context_t *cx, uint32_t u);

#endif //__MACHINE_H
