//
// cfunc.c ... primitive (future overwritten ones)
// Apr 1, 2024 Norihiro Kumagai
//

#include <stdio.h>
#include <string.h>
#include "machine.h"
#include "user.h"
#include "key_in.h"

//
// primitive functions
//

// do_push

void do_push(context_t *cx, word_t value)
{
    cx->sp -= CELLS;
    STAR(cx->sp) = value;
}

word_t do_pop(context_t *cx)
{
    word_t value = STAR(cx->sp);
    cx->sp += CELLS;
    if (STAR(STRICT_ADDR) && cx->sp > DSTACK_END) {
        fprintf(stderr, "stack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_abort(cx, 2, "abort at pop\n");
    }
    return value;
}

// pushr, popr
void do_pushr(context_t *cx, word_t value)
{
    cx->rs -= CELLS;
    STAR(cx->rs) = value;
}

word_t do_popr(context_t *cx)
{
    word_t value = STAR(cx->rs);
    cx->rs += CELLS;
    if (STAR(STRICT_ADDR) && cx->rs > RSTACK_END) {
        fprintf(stderr, "rstack underflow at pc:%04X ip:%04X\n", cx->pc, cx->ip);
        do_abort(cx, 2, "abort at popr\n");
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
    word_t w = getch(cx);
    do_push(cx, w ? w : -1);
}

//
// reset_instream ... for cancel rest of imput-stream
//
void reset_instream(context_t *cx)
{
    //fprintf(stderr, "reset_instream:\n");
    reset_outer();
    mem[STAR(S0_HEAD)] = 0;       // length zero
}

//
// getch/ungetch ... for C function do_word
//
static int uc = -1;

int getch(context_t *cx)
{
    int c, n, i;
    mem_t *p = &mem[STAR(S0_HEAD)];
    if ((c = uc) >= 0) {
        uc = -1;
        return c;
    }
    n = *p;
    i = STAR(IN_ADDR);
    // i should be between 1 and n
    if (i > n)
        return 0;
    c = p[i++];
    STAR(IN_ADDR) = i;
    //fprintf(stderr, "(%c)", c);
    return c;
}

void ungetch(context_t *cx, mem_t c)
{
    uc = c;
}

int lnum = 0;

void do_lnum(context_t *cx)
{
    do_push(cx, lnum);
}

static void dump_line(const char *buf)
{
    int n = strlen(buf);
    char c;
    const char *p = &buf[n > 0 ? n - 1 : 0];
    if (lnum > 0) {
        while (p >= &buf[0] && *p && ((c = *p) == '\n' || c == '\r'))
            --p;
        if (p - buf < n)
            ++p;
        fprintf(stderr, "%d:[%.*s]\n", lnum, (int)(p - buf), buf);
    }
}

//
// do_savefile
//
void do_savefile(const char *path, word_t start, word_t end, word_t last)
{
    FILE *fp = fopen(path, "w");
    const char *p1, *p2;
    size_t len, n;
    word_t header[3];

    if ((fp = fopen(path, "w")) == 0) {
        fprintf(stderr, "savefile: cannot open %s\n", path);
        return;
    }
    
    header[0] = start;
    header[1] = end;
    header[2] = last;

    p1 = (const char *)&mem[start];
    p2 = (const char *)&mem[end];
    len = p2 - p1;
    if (p1 > p2) {
        fprintf(stderr, "savefile: bad address, %04x-%04x\n", start, end);
        goto terminate;
    }
    if (fp == 0) {
        fprintf(stderr,"savefile: cannot open file %s\n", path);
        goto terminate;
    }
    if (fwrite(header, sizeof(word_t), 3, fp) != 3) {
        fprintf(stderr,"savefile: write header error\n");
        goto terminate;
    } else if (fwrite(p1+6, 1, len-6, fp) != len-6) {
        fprintf(stderr,"savefile: write error\n");
        goto terminate;
    }
#if 0 
    else if (fwrite(header, sizeof(word_t), 3, fp) != 3) {
        fprintf(stderr,"savefile: 2nd write header error\n");
        goto terminate;
    }
    n = END_ADDR - DICTTOP_ADDR - 3 * CELLS;
    p1 = &mem[DICTTOP_ADDR];
    if (fwrite(p1, 1, n, fp) != n) {
        fprintf(stderr,"savefile: user variable write error\n");
        goto terminate;
    }
#endif
    n = p2 - p1;
    fprintf(stderr, "savefile: %s, %ld bytes\n", path, n);
terminate: 
    fclose(fp);
}

//
// do_getline: ( -- )
//
int outer_flag = 1;

int do_getline(context_t *cx, char *buf, int size)
{
    // now in-stream buffer emnty, refill it
    //fprintf(stderr, "do_getline: buf = %04x size = %d\n", (word_t)(buf - (char *)&mem[0]), size);
    changemode(0);
    memset(buf, ' ', size-1);
    buf[size-1] = '\0';
    if (outer_flag) {
        if (gets_outer(buf, size) == 0) {
            // outer data expires
            outer_flag = 0;
        }
        if (STAR(DEBUG_ADDR))
            fprintf(stderr, "getline: [%s]", buf);
        lnum++;
    }
    if (outer_flag == 0) {
        // input from keyboard
        if (fgets (buf, size, stdin) == 0) {
            fprintf(stderr, "eof\n");
            changemode(1);
            return EOF;
        }
    }
    changemode(1);
    return 0;
}

//
// do_accept: ( -- )
//
int do_accept(context_t *cx)
{
    static int linecount = 0;
    int n;
    char *buf = &mem[STAR(S0_HEAD)] + 1;

    if (cx->p && cx->rest > 0) {
        return 0;
    }
    if (do_getline(cx, buf, 127 - 1)) {
        return EOF;
    }
    n = strlen(buf);
    for (int i = n - 1; i >= 0 ; --i) {
        if (buf[i] != '\n' && buf[i] != '\r')
            break;
        buf[i] = '\0';
    }
    n = strlen(buf);
    mem[STAR(S0_HEAD)] = n;         // buf length
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "%d [%s]\n", linecount++, buf);
    STAR(IN_ADDR) = 1;  // initial index
    return 0;
}

//
// do_word: (delim -- addr)
//
void do_word(context_t *cx)
{
    char delim, *tib, *p, *here, *h0;
    word_t wp;
    int n, c, count = 0;
    delim = tos(cx);
    here = h0 = &mem[STAR(DP_ADDR)];
    // skip if the first is space
    while (1) {
        if ((c = getch(cx)) == 0) {
            // end-of-file
            tos(cx) = 0;
            return;
        } else if (c == delim) {
            while ((c = getch(cx)) == delim)
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
            if (count++ > 200)
                break;
        } while ((c = getch(cx)) != 0 && c != delim);
        // get a word
        if (((c = h0[1]) == '(' || c == '\\') && here - h0 == 2) {
            //fprintf(stderr, "comment: \n");
            if (c == '(') {
                while ((c = getch(cx)) != 0 && c != ')')
                    ;
                if (c == ')') {
                    // end-of-comment, try the rest of it
                    //fprintf(stderr, "comment: try again\n");
                    here = h0;
                    continue;
                }
            } else if (c == '\\') {
                while ((c = getch(cx)) != 0 && c != '\n')
                    ;
            }
            //fprintf(stderr, "comment: end-of-line\n");
            mem[STAR(DP_ADDR)] = 0;
            tos(cx) = 0;
            return;
         }
         // got a word, exit the loop
         break;
    }

    *here = delim;   // trailing delim char, not counted
    *h0 = here - h0 - 1;        // count byte
    tos(cx) = STAR(DP_ADDR);
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
void do_find(context_t *cx, word_t start)
{
    mem_t *p;
    word_t link = start;
    word_t addr1 = do_pop(cx);
    word_t xt;
    int n3 = mem[addr1], n2, n1;
    //print_cstr(cx, "do_find H", addr1);
    for (; link ; link = prev_entry(link)) {
        //print_cstr(cx, NULL, link);
        n1 = mem[link] & 0x1f;
        if (n1 != n3)
            continue;
        //fprintf(stderr, "find: %04x %d %04x %d (%.*s)\n", addr1, n3, link, n1, n1, &mem[link+1]);
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
    int r;
    int32_t value;
    mem_t *p = &mem[do_pop(cx)];
    mem_t buf[32];
    int count = *p++;
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "number:[%d %.*s]", count, count, p);
    // check if it should convert to double length or not
    r = sscanf(p, "%i", &value) == 1 ? 0 : -1;
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, " -> %d %d\n", value, r);
    // low push first, high second
    do_push(cx, value & 0xffff);
    value >>= 16;
    value &= 0xffff;
    if (value != 0 && value != 0xffff) {
        // double length
        do_push(cx, value);
    }
    do_push(cx, r);
}


void do_prompt(context_t *cx)
{
    if (outer_flag || STAR(STATE_ADDR))
        return;     
    // print only if getline wait for keyboard input
    print_stack(cx);
    if (STAR(STATE_ADDR))
        fprintf(stderr, " compile mode\n");
    else
        fprintf(stderr, " ok\n");
}

// do_emit
void do_emit(context_t *cx, word_t w)
{
    fputc((w&0xff), stdout);
    fflush(stdout);
}

//
// compile process
//
static void compile_it(context_t *cx, word_t w, int flag)
{
    char *p;
    if (STAR(DEBUG_ADDR)&4) {
        if (flag) {
            if (w == STAR(COLON_HEAD))
                p = "\005COLON"; 
            else if (w == STAR(SEMI_HEAD))
                p = "\004SEMI";
            else if (w == STAR(LITERAL_HEAD))
                p = "\005dolit";
            else {
                p = &mem[entry_head(cx, w)];
            }
            fprintf(stderr, "C:%04x %04x (%.*s)\n", STAR(DP_ADDR), w, (*p)&0x1f, p+1);
        } else {
            fprintf(stderr, "C:%04x %04x (%d)\n", STAR(DP_ADDR), w, w);
        }
    }
    STAR(STAR(DP_ADDR)) = w;
    STAR(DP_ADDR) += CELLS;
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
    link_pos = link_addr(STAR(DP_ADDR));
    if (STAR(DEBUG_ADDR)) {
        p = &mem[STAR(DP_ADDR)];
        fprintf(stderr, "C:%04x (%.*s) create\nC:%04x %04x (link)\n", STAR(DP_ADDR), ((*p)&0x1f), p+1, link_pos, prev_link);
    }
    // put link pointer
    STAR(link_pos) = prev_link;
    // uvar last update
    STAR(LAST_ADDR) = STAR(DP_ADDR);
    // user h update
    STAR(DP_ADDR) = link_pos + CELLS;    // allot'ed 
}

// entity of colon word, or machine code "m_start_colondef"
void do_colondef(context_t *cx)
{
    do_create(cx);
    // put COLON xt to cfa
    //fprintf(stderr, "colondef: begin LAST = %04x, HERE = %04x\n", STAR(LAST_ADDR), STAR(DP_ADDR));
    compile_it(cx, STAR(COLON_HEAD), 1);
        // code address should specify "body of machine code"
        // so, xt is not sufficient, one more dereferencing is needed
    // change to compile mode
    STAR(STATE_ADDR) = 1;
    //fprintf(stderr, "colondef: end   HERE = %04x\n", STAR(DP_ADDR));
}

void do_semidef(context_t *cx)
{
    char *p;
    word_t here_addr = STAR(DP_ADDR);
    //fprintf(stderr, "semidef: begin HERE = %04x\n", STAR(DP_ADDR));
    // put EXIT(SEMI) in on-going dictionary entry
    compile_it(cx, STAR(SEMI_HEAD), 1);
    //do_push(cx, STAR(LAST_ADDR));
    //dump_entry(cx);
    // change compile mode
    STAR(STATE_ADDR) = 0;   // interpretive mode
    //fprintf(stderr, "semidef: end   HERE = %04x\n", STAR(DP_ADDR));
}

// do_compile_token
void do_compile_token(context_t *cx)
{
    word_t w = do_pop(cx);
    compile_it(cx, w, 1);
}

void do_compile_number(context_t *cx)
{
    // compile LITERAL and number
    if (STAR(DEBUG_ADDR))
        fprintf(stderr, "compile_number: %04x(%d)\n", tos(cx), tos(cx));
    compile_it(cx, STAR(LITERAL_HEAD), 1);
    compile_it(cx, do_pop(cx), 0);  // compile the number on the stack
}

void do_compile(context_t *cx)
{
    word_t w;
    mem_t *p;
    w= STAR(cx->ip);
    cx->ip += CELLS;
    compile_it(cx, w, 1);
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
    word_t entry = STAR(LAST_ADDR), link, prev = STAR(DP_ADDR);
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
    fprintf(stderr, "%04x .head \"%.*s\"", entry, n, p+1);
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
        //fprintf(stderr, "xt = %04x\n", w);
        if (w == STAR(COLON_HEAD) ) {
            fprintf(stderr, "%04x %04x (colon)\n", ip, w);
        } else if (w == STAR(SEMI_HEAD) ) {
            fprintf(stderr, "%04x %04x (semi)\n", ip, w);
        } else if (w == STAR(LITERAL_HEAD) ) {
            fprintf(stderr, "%04x %04x (dolit)\n", ip, w);
            ip += CELLS;
            w = STAR(ip);
            fprintf(stderr, "%04x %04x (%d)\n", ip, w, w);
        } else {
            entry = entry_head(cx, w);
            p = &mem[entry];
            n = (*p) & 0x1f;
            if ((n == 5 && strncmp(p+1, "dolit", n) == 0) ||
                (n == 7 && strncmp(p+1, "compile", n) == 0) ||
                (n == 6 && strncmp(p+1, "branch", n) == 0) ||
                (n == 7 && strncmp(p+1, "?branch", n) == 0)) {
                ip += CELLS;
                w = STAR(ip);
                fprintf(stderr, "%04x %04x (%d)\n", ip, w, w);
            } else if (n == 7 && strncmp(p+1, "d_dolit", n) == 0) {
                ip += CELLS;
                w = STAR(ip);
                fprintf(stderr, "%04x %04x (%d)\n", ip, w, w);
                ip += CELLS;
                w = STAR(ip);
                fprintf(stderr, "%04x %04x (%d)\n", ip, w, w);
            } else if (n == 7 && strncmp(p+1, "s_dolit", n) == 0) {
                ip += CELLS;
                n = mem[ip];
                p = &mem[ip + 1];
                fprintf(stderr, "%04x .string %d \"%.*s\"\n", ip, n, n, p);
                ip += n + 1 - CELLS;
                //fprintf(stderr, "ip = %04x\n", ip);
            } else {
                fprintf(stderr, "%04x %04x (%.*s)\n", ip, w, n, p+1);
            }
        }
        ip += CELLS;
    }
}

//
// double length integer
//
uint32_t do_dpop(context_t *cx)
{
    word_t high, low;
    high = do_pop(cx);
    low = do_pop(cx);
    return (((uint32_t)high)<<16)|((uint32_t)low);
}

void do_dpush(context_t *cx, uint32_t u)
{
    // push low first, hight next
    do_push(cx, u&0xffff);
    do_push(cx, (word_t)((u>>16)&0xffff));
}

//
// '(quote) as clang primitive
//
int do_quote(context_t *cx)
{
    mem_t *p;
    do_push(cx, ' ');       // push BL
    do_word(cx);            // ( delim --- c-addr )
    p = &mem[tos(cx)];
    do_find(cx, STAR(LAST_ADDR));            // ( c-addr --- 0 | xt flag )
    if (do_pop(cx) == 0) {
        fprintf(stderr, "do_quote: %.*s: not found\n", *p, p+1);
        return -1;
    }
    // now xt on stack
    return 0;
}