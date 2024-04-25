//
// cfunc.c ... primitive (future overwritten ones)
// Apr 1, 2024 Norihiro Kumagai
//

#include <stdio.h>
#include <string.h>
#include "machine.h"

//
// primitive functions
//

// do_push

void do_push(context_t *cx, word_t value)
{
    cx->sp -= 2;
    STAR(cx->sp) = value;
}

word_t do_pop(context_t *cx)
{
    word_t value = STAR(cx->sp);
    cx->sp += CELLS;
    if (cx->sp > DSTACK_END) {
        fprintf(stderr, "stack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    return value;
}

// pushr, popr
void do_pushr(context_t *cx, word_t value)
{
    cx->rs -= CELLS;
    if (cx->rs < 0) {
        fprintf(stderr, "rstack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    STAR(cx->rs) = value;
}

word_t do_popr(context_t *cx)
{
    word_t value = STAR(cx->rs);
    cx->rs += CELLS;
    if (cx->rs > RSTACK_END) {
        fprintf(stderr, "rstack overflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_halt(cx);
    }
    return value;
}

void do_halt(context_t *cx)
{
    cx->halt_flag = 1;
}

void do_dup(context_t *cx)
{
    do_push(cx, STAR(cx->sp));
}

//
// getch for thread code version of accept
//
// ( -- c|-1 )

void do_getch(context_t *cx)
{
    static mem_t buf[128];
    static int i = -1;
    mem_t c;

    while (1) {
        if (i < 0) {
            // read stdin to fill buf        
            if (fgets(buf, 127, stdin) == NULL) {
                fprintf(stderr, "getch: abort, read error\n");
                do_push(cx, -1);
                return;
            }
            fprintf(stderr, "getch: fill[%s]\n", buf);
            i = 0;
        }
        c = buf[i++];
        if (c != 0) {
            do_push(cx, c & 0xff);
            return;
        }
        i = -1;
        // try to read from stdin
    }
}

//
// reset_instream ... for cancel rest of imput-stream
//
void reset_instream(context_t *cx)
{
    //fprintf(stderr, "reset_instream:\n");
    reset_outer();
    cx->p = 0;
    cx->rest = 0;
}

//
// get_instream
//
int get_instream(context_t *cx)
{
    int c;
    if (cx->p == 0 || cx->rest == 0)
        return 0;   // eof
    cx->rest --;
    c = *(cx->p)++;
    //fprintf(stderr, "(%c)", c);
    return c;
}

//
// do_accept: ( -- )
//
int do_accept(context_t *cx)
{
    static int outer_flag = 1;

    if (cx->p && cx->rest > 0) {
        return 0;
    }
    // now in-stream buffer emnty, refill it
    char *buf = &mem[STAR(S0_ADDR)];
    int size = 127, n;
    memset(buf, ' ', size-1);
    buf[size-1] = '\0';
    if (outer_flag) {
        if (gets_outer(buf, size - 1) == 0) {
            // outer data expires
            outer_flag = 0;
        }
    }
    if (outer_flag == 0) {
        // input from keyboard
        do_prompt(cx);
        if (fgets (buf, size - 1, stdin) == 0) {
            fprintf(stderr, "eof\n");
            return EOF;
        }
    }
    n = strlen(buf);
    while (0 < n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) {
        buf[--n] = ' ';
    }
    if (buf[n - 1] != ' ' && n < 79) {
        buf[n] = ' ';
        buf[n + 1] = '\0';
    }
    cx->p = buf;
    cx->rest = n;
    return 0;
}

//
// do_word: (delim -- addr)
//
void do_word(context_t *cx)
{
    char delim, *tib, *p, *here, *h0;
    word_t wp;
    int n, c;
    delim = tos(cx);
    here = h0 = &mem[STAR(H_ADDR)];
    // skip if the first is space
    while (1) {
        if ((c = get_instream(cx)) == 0) {
            // end-of-file
            tos(cx) = 0;
            return;
        } else if (c == delim) {
            while ((c = get_instream(cx)) == delim)
                ;   // skip delimiters
            if (c == 0) {
                tos(cx) = 0;
                return;
            }
        }
        // now c has neither delim nor eof
        here++;     // keep the first count char
        do {
            *here++ = c;
            //fprintf(stderr, "[%c]", c);
        } while ((c = get_instream(cx)) != 0 && c != delim);
        // get a word
        if (((c = h0[1]) == '(' || c == '\\') && here - h0 == 2) {
            //fprintf(stderr, "comment: \n");
            if (c == '(') {
                while ((c = get_instream(cx)) != 0 && c != ')')
                    ;
                if (c == ')') {
                    // end-of-comment, try the rest of it
                    //fprintf(stderr, "comment: try again\n");
                    here = h0;
                    continue;
                }
            } else if (c == '\\') {
                while ((c = get_instream(cx)) != 0 && c != '\n')
                    ;
            }
            //fprintf(stderr, "comment: end-of-line\n");
            mem[STAR(H_ADDR)] = 0;
            tos(cx) = 0;
            return;
         }
         // got a word, exit the loop
         break;
    }

    *here = delim;   // trailing delim char, not counted
    *h0 = here - h0 - 1;        // count byte
    tos(cx) = STAR(H_ADDR);
}

static word_t link_addr(word_t entry)
{
    mem_t *p;
    word_t link;
    int n;
    p = &mem[entry];
    n = *p & 0x1f;
    return entry + n + (n & 1) + 2;
}

static word_t prev_entry(word_t link)
{
    link = STAR(link_addr(link));
    return link;
}

word_t code_addr(word_t entry)
{
    word_t addr = link_addr(entry);
    return addr + CELLS;
}

static word_t param_addr(word_t entry)
{
    word_t addr = link_addr(entry);
    return addr + 4;
}

// do_compare ... adopted from dpan94
// COMPARE ( c-addr1 u1 c-addr2 u2 -- n)

// Compare the string specified by c-addr1 u1 to the string specified
// by c-addr2 u2. The strings are compared, beginning at the given 
// addresses, character by character, up to the length of the shorter
// string or until a difference is found. If the two strings are 
// identical, n is zero. If the two strings are identical up to the
// length of the shorter string, n is minus-one (-1) if u1 is less 
// than u2 and one (1) otherwise. If the two strings are not
// identical up to the length of the shorter string, n is
// minus-one (-1) if the first non-matching character in the string
// specified by c-addr1 u1 has a lesser numeric value than the 
// corresponding character in the string specified by c-addr2 u2 and
// one (1) otherwise

// c-addr1 u1で指定された文字列と、c-addr2 u2で指定された文字列を比較する。
// 文字列は、指定されたアドレスから始まり、一文字ずつ、短い方の文字列の長さまで、
// または違いが見つかるまで比較される。もし2つの文字列が同一の場合、nは0である。
// 2つの文字列が短い方の文字列の長さまで同じ場合、nは、u1がu2より小さければ
// マイナス1（-1）、そうでなければ1（1）である。2つの文字列が短い方の文字列の
// 長さまで同じでない場合、nは、c-addr1 u1で指定される文字列の最初の文字が
// 一致しない場合はマイナス1 (-1)となる。c-addr1 u1 で指定される文字列の
// 最初のマッチしない文字が、 c-addr2 u2 で指定される文字列の対応する文字
// よりも小さい数値を持つ場合、 n はマイナス1 (-1) となり、そうでない
// 場合は1 (1) となる。
void do_compare(context_t *cx)
{
    // ( c-addr1 u1 c-addr2 u2 -- n)
    word_t u1, u2, w1, w2;
    mem_t *p1, *p2;
    int result, n;

    u2 = do_pop(cx);
    w2 = do_pop(cx);
    p2 = &mem[w2];
    u1 = do_pop(cx);
    w1 = do_pop(cx);
    p1 = &mem[w1];
    // compare logically
    n = u1 < u2 ? u1 : u2;
    result = strncmp(p1, p2, n);
    if (result > 0)
        result = 1;
    if (result < 0)
        result = -1;
    if (result == 0) {
        // one string may includes the other,
        // in such a case, they do not "equal"
        if (u1 < u2)
            result = -1;
        else if (u1 > u2)
            result = 1;
    }
    //fprintf(stderr, "compare: [%d %.*s][%d %.*s] -> %d\n", u1, u1, p1, u2, u2, p2, result);
    do_push(cx, result);
}

// do_find: find a entry whose name is the same
//   as top-of-directory
// (c-addr -- 0     (not found)
//            xt 1  (find, normak)
//            xt -1 (find, immediate))
void do_find(context_t *cx)
{
    mem_t *p;
    word_t link = STAR(LAST_ADDR);
    word_t addr1 = do_pop(cx);
    word_t xt;
    int n3 = mem[addr1], n2, n1;
    //print_cstr(cx, "do_find H", addr1);
    for (; link ; link = prev_entry(link)) {
        //print_cstr(cx, NULL, link);
        n1 = mem[link] & 0x1f;
        if (n1 != n3)
            continue;
        //fprintf(stderr, "find: %04x %d %04x %d\n", addr1, n3, link, n1);
        do_push(cx, addr1+1);
        do_push(cx, n3);
        do_push(cx, link+1);
        do_push(cx, n1);
        do_compare(cx);
        n2 = do_pop(cx);
        if (n2)
            continue;
        // match
        break;
    }
    if (link == 0) {
        do_push(cx, 0);
    } else {
        xt = code_addr(link);
        do_push(cx, xt);
        if (mem[link] & 0x80)
            do_push(cx, -1);
        else
            do_push(cx, 1);
    }
    //print_stack(cx);
}

//
// do_number (addr -- n r)
// addrのカウント付き文字列を数値に変換する。BASEは見ない。sscanfの%i変換に
// 依る。すなわち、先頭が 0x または 0Xの場合は16進数変換、0の場合は8進変換、
// その他は10進変換となる。
// 結果はnとして置かれ、その上に変換結果rが返される。rが0の場合は変換成功
// である。
void do_number(context_t *cx)
{
    int value, r;
    mem_t *p = &mem[do_pop(cx)];
    int count = *p++;
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "number:[%d %.*s]", count, count, p);
    r = sscanf(p, "%i", &value) == 1 ? 0 : -1;
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, " -> %d %d\n", value, r);
    do_push(cx, value);
    do_push(cx, r);
}


void do_prompt(context_t *cx)
{
    print_dstack(cx);
    fprintf(stderr, " ok\n");
}

// do_emit
void do_emit(context_t *cx, word_t w)
{
    fputc((w&0xff), stdout);
}

// do_create ( -- ) ... no stack operation
// パラメータ・フィールドのメモリを確保することなく、
// <name>の辞書エントリが作成される。その後、<name>が実行されると、
// <name>のパラメータ・フィールドの最初のバイトのアドレスがスタックに
// 残される。

// name string, link pointers are initialized, with user variable 
// 'h' alloted, 'last' remains to point top of this brand-new entry.

void do_create(context_t *cx)
{
    mem_t *p, *q;
    word_t w, entry_pos, prev_link, link_pos;
    int count, n;
    // read a word to put it at the last of dictionary, pointed last, here
    do_push(cx, ' ');   // push a space as delimiter for do_word
    do_word(cx);
    // clear stack
    do_pop(cx); 
    // check its name
    prev_link = STAR(LAST_ADDR);
    link_pos = link_addr(STAR(H_ADDR));
    //fprintf(stderr, "create: head = %04x, link_pos - %04x\n", STAR(H_ADDR), link_pos);
    // put link pointer
    STAR(link_pos) = prev_link;
    //fprintf(stderr, "prev_link: %04x to %04x\n", prev_link, link_pos);
    // uvar last update
    STAR(LAST_ADDR) = STAR(H_ADDR);
    // user h update
    STAR(H_ADDR) = link_pos + CELLS;    // allot'ed 
}

// entity of colon word, or machine code "m_start_colondef"
void do_start_colondef(context_t *cx)
{
    do_create(cx);
    // put COLON xt to cfa
    //fprintf(stderr, "start_colondef: begin LAST = %04x, HERE = %04x\n", STAR(LAST_ADDR), STAR(H_ADDR));
    STAR(STAR(H_ADDR)) = STAR(STAR(COLON_ADDR));
        // code address should specify "body of machine code"
        // so, xt is not sufficient, one more dereferencing is needed
    STAR(H_ADDR) += CELLS;      // allot'ed
    // change to compile mode
    STAR(STATE_ADDR) = 1;
    //fprintf(stderr, "start_colondef: end   HERE = %04x\n", STAR(H_ADDR));
}

void do_end_colondef(context_t *cx)
{
    char *p;
    word_t here_addr = STAR(H_ADDR);
    //fprintf(stderr, "end_colondef: begin HERE = %04x\n", STAR(H_ADDR));
    // put EXIT(SEMI) in on-going dictionary entry
    STAR(here_addr) = STAR(SEMI_ADDR);  // put SEMI xt
    STAR(H_ADDR) += CELLS;
    // change compile mode
    STAR(STATE_ADDR) = 0;   // interpretive mode
    //fprintf(stderr, "end_colondef: end   HERE = %04x\n", STAR(H_ADDR));
}

#if 0
// entity of colon word, or machine code "m_start_colondef"
void do_constant(context_t *cx)
{
    word_t w = do_pop(cx);  // const value
    do_create(cx);
    // put COLON xt to cfa
    //fprintf(stderr, "constant: begin LAST = %04x, HERE = %04x\n", STAR(LAST_ADDR), STAR(H_ADDR));
    STAR(STAR(H_ADDR)) = STAR(STAR(DOCONS_ADDR));
        // code address should specify "body of machine code"
        // so, xt is not sufficient, one more dereferencing is needed
    STAR(H_ADDR) += CELLS;      // allot'ed
    // change to compile mode
    STAR(STAR(H_ADDR)) = w;     // set constant 
    STAR(H_ADDR) += CELLS;      // allot'ed
    //fprintf(stderr, "constant: end   HERE = %04x\n", STAR(H_ADDR));
}
#endif

// do_compile_token
void do_compile_token(context_t *cx)
{
    word_t xt = tos(cx);
    word_t entry = entry_head(cx, xt);
    char *p = &mem[entry];
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "C:%04x %04x (%.*s)\n", STAR(H_ADDR), xt, (*p)&0x1f, p+1);
    STAR(STAR(H_ADDR)) = do_pop(cx);
    STAR(H_ADDR) += CELLS;
}

void do_compile_number(context_t *cx)
{
    // compile LITERAL and number
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "compile_number:\n");
    do_push(cx, STAR(LITERAL_ADDR));
    do_compile_token(cx);
    do_compile_token(cx);    // compile the number on the stack
}

void do_compile(context_t *cx)
{
    word_t w;
    mem_t *p;
    w= STAR(cx->ip);
    cx->ip += CELLS;
    STAR(STAR(H_ADDR)) = w;
    p = &mem[entry_head(cx, w)];
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "C:%04X %04X (%.*s)\n", STAR(H_ADDR), w, (*p)&0x1f, (p+1));
    STAR(H_ADDR) += CELLS;
}

void do_bracompile(context_t *cx)
{
    int flag;
    fprintf(stderr, "[COMPILE]: \n");
    while (1) {
        do_push(cx, ' ');
        do_word(cx);
        if (tos(cx) != 0)
            break;
        do_pop(cx); // discard it
        if (do_accept(cx) == EOF)
            do_abort(cx, "need a word");
        print_s0(cx);
    }
    //print_cstr(cx, "H", STAR(H_ADDR));
    //print_stack(cx);
    do_find(cx);
    //if (do_pop(cx))
    //    do_pop(cx); // clear the result of do_find
    //continue;
    if ((flag = do_pop(cx)) == 0)
        do_abort(cx, "not find in [COMPILE]");
    do_compile_token(cx);
}

//
// dictionary entry handling
//

word_t entry_head(context_t *cx, word_t addr)
{
    word_t entry = STAR(LAST_ADDR), link;
    mem_t *p;
    while (addr < entry) {
        link = link_addr(entry);
        if ((entry = STAR(link)) == 0)
            break;
        //fprintf(stderr, "[%d %.*s]", ((*p)&0x1f),((*p)&0x1f),(p+1));
    }
    return entry;
}

word_t entry_tail(context_t *cx, word_t addr)
{
    word_t entry = STAR(LAST_ADDR), link, prev = STAR(H_ADDR);
    mem_t *p;
    while (addr < entry) {
        link = link_addr(entry);
        prev = entry;
        //fprintf(stderr, "addr = %04X, entry = %04X\n", addr, entry);
        if ((entry = STAR(link)) == 0)
            break;
        //fprintf(stderr, "[%d %.*s]", ((*p)&0x1f),((*p)&0x1f),(p+1));
    }
    return prev;
}

// dump entry (addr -- )
void dump_entry(context_t *cx)
{
    // dump entry
    word_t entry, link, tail, ip, w;
    const mem_t *p;
    int n;

    entry = entry_head(cx, w = do_pop(cx));
    tail = entry_tail(cx, w);
    fprintf(stderr, "dump_entry: entry = %04X, tail = %04X\n", entry, tail);
    link = link_addr(entry);
    //fprintf(stderr, "dump_last_entry: last = %04x, link = %04x(%04x), here = %04x\n", last, link, STAR(link), here);
    // dump it
    p = &mem[entry];
    n = (*p) & 0x1f;
    fprintf(stderr, "%04x .head [%.*s]", entry, n, p+1);
    if ((*p) & 0xe0)
        fprintf(stderr, "[%02x]", (*p) & 0xe0);
    fprintf(stderr, "\n");
    ip = link;
    fprintf(stderr, "%04x %04x  [link]\n", ip, STAR(ip));
    ip += CELLS;
    fprintf(stderr, "%04x %04x  [code]\n", ip, STAR(ip));
    ip += CELLS;
    while (ip < tail) {
        w = STAR(ip);
        entry = entry_head(cx, w);
        if (entry) {
            p = &mem[entry];
            n = (*p) & 0x1f;
            fprintf(stderr, "%04x %04x (%.*s)\n", ip, w, n, p+1);
            if ((n == 7 && strncmp(p+1, "literal", n) == 0) ||
                (n == 6 && strncmp(p+1, "branch", n) == 0) ||
                (n == 7 && strncmp(p+1, "?branch", n) == 0)) {
                ip += CELLS;
                w = STAR(ip);
                fprintf(stderr, "%04x %04x (%d)\n", ip, w, w);
            }
        }
        ip += CELLS;
    }
}

