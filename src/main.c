//
// main.c ... cpnr main functions
// March 24, 2024  Norihiro Kumagai

#include <stdio.h>
#include <inttypes.h>

typedef uint16_t word_t;

#define MAX_VARIABLE 20

typedef struct _ctx {
    jmp_buf env;
    word_t ip;      // instruction pointer
    word_t wa;      // working address
    word_t ca;      // code address
    word_t rs;      // return stack pointer
    word_t sp;      // data stack pointer
    word_t ah;
    word_t al;      // accumulator high, low
    word_t var[MAX_VARIABLE]; // user variable array
    word_t stack[256];
    word_t rstack[256];
} context_t;

#define HERE 1
#define BASE 2
#define STATE 3
#define LAST 4

//
//  counted-stringをC言語文字列にコピーする
//
char *str(void *c_str)
{
    static char buf[256];
    char *p = (char *)c_str;
    int n = *p;
    n = (n < 255) ? n : 255;
    memcpy (buf, p + 1, n);
    p[n] = '\n'
    return buf;
}

void do_catch(context_t *cx)
{
    int result;
    result = setjmp(cx->env);    // no disclimination
}

// do_abort
// If the flag is true, types out the last word interpreted, followed by the
// text. Also clears the user's stacks and returns control to the terminal. If
//false, takes no action.
void do_abort(context_t *cx, char *mes)
{
    char *p;
    if (tos(cx)) {
        p = str(cx->var[LAST]);      // entry top, word name
        fprintf(stderr, "%s %s\n", p, mes);
    }
    longjmp(cx->env, tos(cx));
}

// do_push

void do_push(context_t *cx, word_t value)
{
    cx->stack[--(cx->sp)] = value;
}

void do_dup(context_t *cx, word_t value)
{
    do_push(cx, cx->stack[cx->sp]);
}

word_t tos(context_t *cx)
{
    return cx->stack[cx->sp];
}

// do_accept: read one line

// do_word: cut a word to top-of-dictionary

// do_find: find a entry whose name is the same
//   as top-of-directory
// (c-addr -- 0     (not found)
//            xt 1  (find, normak)
//            xt -1 (find, immediate))

// do_execute: invoke xt on tos
// (xt -- )
// xt actually is a code-address of the dict-entry

// innter interpreter
//

void m_colon(context_t *cx)
{
    m_pushr(cx, cx->ip);
    cx->ip = cx_wa;
    // jmp next
    m_next(cx);
}

void m_semi(context_t *cx)
{
    cx->ip = do_popr(cx);
    m_next(cx);
}

void m_next(context_t *cx)
{
    cx->wa = RAM(cx, cx->ip);
    cx->ip += 2;
    // jmp run
    m_run(cx);
}

void m_run(context_t *cx)
{
    cx->ca = RAM(cx, cx->wa);
    cx->wa += 2;
    //cx->pc = cx->ca;
    m_jmp(cx, cx->ca)
}

void do_execute (context_t *cx)
{
    // start innter interpreter
    cx->wa = do_pop(cx);
    m_run(cx);
    machine_code(cx);       // run infinite loop 
}

void machine_code(context_t *cx) 
{
    // do infinite loop 
    while (1) {
        // fetch next code
        code = RAM[cx->pc];
        perform_code(cx, code);
    }
}

void perform_code(context_t *cx, word_t code)
{
    // do one instruction
    word_t inst = code & 0xf000;
    if (inst != 0xc000) {
        // relative jump
        fprintf(stderr, "%04x: %04x bad code\n", cx->pc, code);
        do_push(cx, 39);
        do_abort(cx, NULL);
        // not fall down
    }
    // machine code
    switch(code & 0xfff) {
    case 1: // colon
        do_pushr(cx, cx->ip);
        cx->ip = cx->wa;
    case 2: // next
    m_next:
        cx->wa = RAM(cx, cx->ip);
        cx->ip += 2;
    case 3: // run
        cx->ca = RAM(cx, cx->wa);
        cx->wa += 2;
        cx->pc = cx->ca;
        break;
    case 4: // semi
        cx->ip = do_popr(cx);
        break;
    case 5: // conditional bra in thread
        if (tos(cx) == 0) {
            cx->ip += 2;       // skip branch operand
            break;
        }
    case 6: // unconditional bra in thread
        cx->ip = RAM(cx, cx->ip + 2);
        break;
    default:
        // relative jump
        fprintf(stderr, "%04x: %04x bad code\n", cx->pc, code);
        do_push(cx, 39);
        do_abort(cx, NULL);
    }
    cx->pc += 2;
final:
    fprintf(stderr, "%04x IP:%04x WA:%04x CA:%04x AH:%04x AL:%04x ",
            cx->pc, cx->ip, cx->wa, cx->ca, cx->ah, cx->al);
    for (int i = 254; i >= cx->sp; i -= 2) {
        fprintf(srderr, " %04X");
    }
    fprintf(stderr, "\n");
    getchar();
}

int main (int ac, char **av)
{
    // outer interpreter
    char buf[80];
    int n;
    context_t _ctx, *cx;

    // initialize ctx
    while (1) {
        cx = &_ctx;
        do_catch(cx);
        initialize_ctx(cx);
        do_accept(cx);
        while (1) {
            do_push(cx, ' ');     // push delimiter
            do_word(cx);
            if (tos(cx) == 0) {
                continue;
            }
            do_dup(cx);
            do_find(cx);
            if (top(cx) != 0) {
                do_execute(cx);
            } else {
                do_number(cx);
                do_dup(cx);
                if (tos(cx) != 0) {
                    do_abort(" not found\n");
                }
            }
        }
    }
}

