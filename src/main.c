//
// main.c ... cpnr main functions
// March 24, 2024  Norihiro Kumagai

#include <stdio.h>
#include <string.h>
#include "machine.h"
#include "user.h"

//
// forth interpreter initializer
//

void reset(context_t *cx)
{
    cx->pc = 0;
}

static void initialize_ctx(context_t *cx)
{
    cx->ip = 0;
    cx->wa = 0;
    cx->ca = 0;
    cx->rs = RSTACK_END;
    cx->sp = DSTACK_END;
    reset(cx);      // initialize cx->pc
    cx->halt_flag = 0;
    cx->ss_flag = 0;
    for (int i = 0; i < BPTBL_SIZE; ++i)
        cx->bp[i] = 0;
    lnum = 0;
}

//
// 
//
static char **filenames;
static int filenames_length;
static FILE *fp;

void init_outer_buf(int ac, char **av)
{
    filenames = av;
}

void reset_outer(void)
{
    if (fp) {
        fclose(fp);
        fp = 0;
    }
    filenames = 0;
}

int gets_outer(mem_t *buf, int len)
{
    int c;
    while (1) {
        if (filenames == 0 || filenames[0] == 0) {
            filenames = 0;
            return 0;
        }
        if (fp == 0) {
            fprintf(stderr, "open: %s\n", filenames[0]);
            if ((fp = fopen(filenames[0], "r")) == 0) {
                fprintf(stderr, "gets_outer: cannot open %s, no file input any more\n", filenames[0]);
                filenames = 0;
                return 0;
            }
            // now file input established
            lnum = 1;
        }
        if (fgets(buf, len-1, fp) != 0) {
            //fprintf(stderr, "gets_outer: [%s]\n", buf);
            return strlen(buf); // ok, read a line and return
        }
        // try to open next file
        fclose(fp);
        fp = 0;
        filenames++;
        // loop again, open next file and try to read the 1st line
    }
}
//
//  counted-stringをC言語文字列にコピーする
//
char *str(mem_t *c_str)
{
    static char buf[256];
    char *p = (mem_t *)c_str;
    int n = *p;
    n = (n < 255) ? n : 255;
    memcpy (buf, p + 1, n);
    p[n] = '\0';
    return buf;
}

//
// dictdump
//



//
// init_dict
//

// to_hex(int c)
// return its hex value if the char c is one of hex chars,
// otherwise return -1
static int to_hex(int c)
{
    static char *lower_hex = "0123456789abcdef";
    static char *upper_hex = "0123456789ABCDEF";
    const char *p;
    if ((p = index(lower_hex, c))) {
        return p - lower_hex;
    }
    if ((p = index(upper_hex, c))) {
        return p - upper_hex;
    }
    return -1;
}

//
// fgethex(fp)
// read hex string to convert it in positive hex value,
// otherwise return -1
static int fgethex(FILE *fp)
{
    int value = 0, c = -1, n = -1, valid_flag = 0;
    while ((c = fgetc(fp)) != EOF && (n = to_hex(c)) >= 0) {
        valid_flag = 1;
        value = value * 16 + n;
    }
    if (c != EOF && n < 0)
        ungetc(c, fp);
    if (valid_flag)
        return value;
    else 
        return -1;
}

static int filetype(const char *name)
{
    char *p = rindex(name, '.');
    if (p) {
        p++;
        if (strcmp(p, "bin") == 0)
            return 1;
        else if (strcmp(p, "X") == 0)
            return 2;
    }
    return 0;
}

static int read_xfile(FILE *fp)
{
    int c;
    int i, n, value, min = 0xffff, max = 0;
    word_t addr;

    while ((c = fgetc(fp)) != EOF) {
        if (c == ' ' || c == '\r' || c == '\n')
            continue;
        if (c == '=') {
            // do address
            if ((value = fgethex(fp)) == -1)
                continue;
            if (value < 0 || MEMSIZE <= value)
                continue;
            if (addr != value) {
                //fprintf(stderr, "addr value = %04x\n", value);
                addr = value;
            }
            // min, max
            if (addr < min)
                min = addr;
            if (max < addr)
                max = addr;
        }
        if ((i = to_hex(c)) >= 0) {
            n = 1;
            value = i;
            while ((c = fgetc(fp)) != EOF && (i = to_hex(c)) >= 0) {
                value = value * 16 + i;
                n++;
            }
            if (c != EOF && i < 0)
                ungetc(c, fp);
            if (n > 2) {
                //fprintf(stderr, "[%04X %04X]\n", addr, value);
                STAR(addr) = value; // word write
                addr += 2;
            } else {
                //fprintf(stderr, "[%04X %02X]\n", addr, value);
                mem[addr] = value;  // byte write
                addr++;
            }
        }
    }
#if 0
    int first = 1;
    fprintf(stderr, "init_dict: min = %04x, max=%04x\n", min, max);
    for (int i = min; i <= max; i += 2) {
        if (first || (i % 16) == 0) {
            fprintf(stderr, "%04X: ", i);
            first = 0;
        }
        fprintf(stderr, "%04X ", STAR(i));
        if ((i % 16) == 14)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif
    // init user vars
    STAR(DICTTOP_ADDR) = STAR(min);
    STAR(DP_ADDR) = STAR(min + 2);
    STAR(LAST_ADDR) = STAR(min + 4);
    fprintf(stderr, "dicttop: %04x, last: %04x, h: %04x\n", STAR(DICTTOP_ADDR), STAR(LAST_ADDR), STAR(DP_ADDR));
    return 0;
}

static int init_dict(context_t *cx, const char *filename)
{
    char *p;
    FILE *fp;
    int c, n, i, type, result;
    word_t addr = 0, *wp;
    word_t header[3];

    if (filename == 0) {
        fprintf(stderr, "no filename\n");
        return -1;

    }
    if ((type = filetype(filename)) == 0) {
        fprintf(stderr, "unexpected file type: %s\n", filename);
        return -1;
    }
    if ((fp = fopen(filename, "r")) == 0) {
        fprintf(stderr, "open error in init_dict: %s\n", filename);
        return -1;
    }
    if (type == 1) {     // bin file
        n = sizeof header / sizeof (word_t);
        if ((result = fread(&header, sizeof(word_t), n, fp)) != n) {
            fprintf(stderr, "bin file, header read error\n");
            fclose(fp);
            return -1;
        }
        // header[0] ... DICTTOP
        // header[1] ... DP
        // header[2] ... LAST
        fprintf(stderr, "%s: dicttop = %04x, dp = %04x, last = %04x\n", filename, header[0], header[1], header[2]);
        p = &mem[header[0]];
        n = header[1] - header[0];
        fseek(fp, 0L, SEEK_SET);
        fprintf(stderr, "n = %x(%d)\n", n, n);
        if (fread(p, 1, n, fp) != n) {
            fprintf(stderr, "%s: dict read error\n", filename);
            fclose(fp);
            return -1;
        }
        STAR(DICTTOP_ADDR) = header[0];
        STAR(DP_ADDR) = header[1];
        STAR(LAST_ADDR) = header[2];
        fclose(fp);
        return 0;
    } else if (type == 2) {
        fprintf(stderr, "%s: read_xfile\n", filename);
        read_xfile(fp);
        fclose(fp);
        return 0;
    } else {
        return -1;
    }
}

// set xt to a user var
// name2xt ( -- xt), uses C arg 'name'
static int name2xt(context_t *cx, char *name)
{
    char *p;
    int n;
    word_t w;
    word_t cstr_addr = STAR(DP_ADDR);
    // get 'name' entry address
    p = &mem[cstr_addr];
    *p++ = n = strlen(name);
    strncpy(p, name, n);
    p[n] = ' ';
    do_push(cx, cstr_addr);
    //fprintf(stderr, "find: [%.*s]\n", mem[cstr_addr], p);
    do_find(cx);
    if (do_pop(cx) == 0) {
        fprintf(stderr, "name2xt: %s: no entry, error\n", name);
        return -1;
    }
    //w = code_addr(do_pop(cx));
    //do_push(cx, w);
    return 0;
}

// initialize mem[] array, mainly user variables

static int init_mem(context_t *cx)
{
    mem_t *p;
    int flag = 0;
    // DICTTOP has already been set, no need to care here
    STAR(S0_ADDR) = DSTACK_END;  // s0 line buffer
    STAR(R0_ADDR) = RSTACK_END;
    STAR(TIB_ADDR) = TIB_START;
    STAR(STATE_ADDR) = 0;    // interpretive mode
    STAR(BASE_ADDR) = 10;     // DECIMAL mode
    flag |= name2xt(cx, "halt");
    STAR(HALT_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "colon");
    STAR(COLON_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "semi");
    STAR(SEMI_ADDR) = do_pop(cx);
    flag |= name2xt(cx, "dolit");
    STAR(LITERAL_ADDR) = do_pop(cx);
    STAR(ABORT_ADDR) = 0;
    if (name2xt(cx, "abort") == 0) {
        STAR(ABORT_ADDR) = do_pop(cx);
        fprintf(stderr, "abort_addr: %04x\n", STAR(ABORT_ADDR));
    }
    STAR(COLD_ADDR) = 0;
    if (name2xt(cx, "cold") == 0)
        STAR(COLD_ADDR) = do_pop(cx);
    STAR(DEBUG_ADDR) = 0;
    STAR(PAD_ADDR) = STAR(DP_ADDR);
    STAR(IN_ADDR) = 0;
    //flag |= name2xt(cx, "docons");
    //STAR(DOCONS_ADDR) = do_pop(cx);
    if (flag) {
        return -1;
    }
    //STAR(DEBUG_ADDR) = 1;
    if (STAR(DEBUG_ADDR)) {
        fprintf(stderr, "init_mem: initial DEBUG_ADDR = %d\n", STAR(DEBUG_ADDR));
    }
    //fprintf(stderr, "init_mem: halt xt = %04X, semi xt = %04X\n", STAR(HALT_ADDR), STAR(SEMI_ADDR));
    return 0;
}

int main (int ac, char **av)
{
    // outer interpreter
    char buf[80];
    int n;
    word_t abort_addr;
    context_t _ctx, *cx;

    // init source file args
    // initialize ctx
    cx = &_ctx;
    initialize_ctx(cx);
    if (init_dict(cx, av[1]) != 0) {
        fprintf(stderr, "exit init_dict error\n");
        return 1;
    }
    init_outer_buf(ac - 2, av + 2);
    init_optable();
    if (init_mem(cx) < 0) {
        fprintf(stderr, "exit init_mem error\n");
        return 1;
    }
    abort_addr = STAR(ABORT_ADDR);
    if (abort_addr) {
        fprintf(stderr, "start abort at %04x\n", abort_addr);
        do_push(cx, abort_addr);
        // STAR(DEBUG_ADDR) = 1;
        do_execute(cx);
    } else {
        fprintf(stderr, "start text interpreter\n");
        do_mainloop(cx);
    }
    return 0;
}

