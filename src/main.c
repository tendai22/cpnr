// main.c ... cpnr main functions
// March 24, 2024  Norihiro Kumagai
//

#include <stdio.h>
#include <string.h>
#include "machine.h"
#include "user.h"
#include "key_in.h"

word_t org_addr;
word_t user_org_addr;

static int plain_flag = 0;

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
    fprintf(stderr, "reset_outer:\n");
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
        else if (strcmp(p, "hex") == 0)
            return 3;
    }
    return 0;
}

static int read_hexfile(FILE *fp)
{
    int c, bcount, addr, rtype, b, seg = 0, eof_flag = 0;
    word_t min, max;
    min = 0xffff;
    max = 0;
    do {
        // skip until start code
        while ((c = fgetc(fp)) != EOF && c != ':')
            ;       // skip until start code
        if (c == EOF)
            break;
        // byte count
        bcount = to_hex(fgetc(fp));
        bcount *= 16;
        bcount += to_hex(fgetc(fp));
        // address field
        addr = 0;
        for (int i = 0; i < 4; ++i) {
            addr = addr * 16 + to_hex(fgetc(fp));
        }
        // record type
        rtype = to_hex(fgetc(fp));
        rtype *= 16;
        rtype += to_hex(fgetc(fp));
        // data, bcount times
        switch(rtype) {
        case 0: // data
            fprintf(stderr, "%04x: (%d) ", addr, bcount);
            if (addr < min) {
                min = addr;
                fprintf(stderr, "min = %04x\n", min);
            }
            while (bcount-- > 0) {
                b = to_hex(fgetc(fp));
                b *= 16;
                b += to_hex(fgetc(fp));
                mem[addr++] = b;
                fprintf(stderr, "%02x ", b);
            }
            fprintf(stderr, "\n");
            if (max < addr)
                max = addr;
            break;
        case 1: // end-of-file
            fprintf(stderr, "eof\n");
            eof_flag = 1;
            break;
        default:
            fprintf(stderr, "read_hexfile: unknown record type %02x, ignore\n", rtype);
            break;
        }
        // follow to next 'colon search'
    }  while (eof_flag == 0 && feof(fp) == 0 && ferror(fp) == 0);
    if (ferror(fp) != 0) {
        fprintf(stderr, "read_hextfile: ferror\n");
    }
    return ferror(fp) ? -1 : min;
}

static int read_xfile(FILE *fp)
{
    int c;
    int i, n, value, min = 0xffff, max = 0;
    mem_t *src, *dest;
    word_t addr, offset;

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
    // init org_addr
    org_addr = min;
    // init user vars
    offset = min - STAR(min);   // usually zero
    fprintf(stderr, "read_xfile: offset = %04x\n", offset);
    return min;
}

static void init_org(context_t *cx, word_t top)
{

    org_addr = STAR(top);
    user_org_addr = STAR(org_addr + UP_HEAD - DICTTOP_HEAD);
    fprintf(stderr, "init_org: org_addr = %04x, user_org_addr = %04x\n", org_addr, user_org_addr);
}

static int read_dict(context_t *cx, const char *filename)
{
    char *p;
    FILE *fp;
    int c, n, i, type, result;
    word_t addr = 0, org, *wp;
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
        //fprintf(stderr, "n = %x(%d)\n", n, n);
        if (fread(p, 1, n, fp) != n) {
            fprintf(stderr, "%s: dict read error\n", filename);
            fclose(fp);
            return -1;
        }
        fclose(fp);
        org = header[0];
        //
        return org;
    } else if (type == 2) {
        fprintf(stderr, "%s: read_xfile\n", filename);
        org = read_xfile(fp);
        fclose(fp);
        return org;
    } else if (type == 3) {
        fprintf(stderr, "%s: read_hexfile\n", filename);
        org = read_hexfile(fp);
        fclose(fp);
        return org;
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
    do_find(cx, STAR(LAST_ADDR));
    if (do_pop(cx) == 0) {
        fprintf(stderr, "name2xt: %s: no entry\n", name);
        return -1;
    }
    //w = code_addr(do_pop(cx));
    //do_push(cx, w);
    return 0;
}

// initialize mem[] array, mainly user variables

static int init_mem(context_t *cx, word_t org)
{
    word_t user;
    mem_t *src, *dest;
    int size, flag = 0;
    // org is not specified, 
    fprintf(stderr, "init_mem: org: %04x\n", org);
    // user check
    org_addr = org;
    // DP, LAST
    if (plain_flag == 0 && STAR(DP_HEAD) == 0)
        STAR(DP_HEAD) = STAR(DICTEND_HEAD);
    if (plain_flag == 0 && STAR(LAST_HEAD) == 0)
        STAR(LAST_HEAD) = STAR(DICTENTRY_HEAD);
    fprintf(stderr, "init_mem: dp: %04x, last: %04x\n", STAR(DP_HEAD), STAR(LAST_HEAD));
    // set S0, R0, TIB
    if (plain_flag == 0 && STAR(UP_HEAD) == 0)
        STAR(UP_HEAD) = 0xf000;
    user = STAR(UP_HEAD);
    fprintf(stderr, "init_mem: user: %04x\n", user);
    user_org_addr = user;
    if (plain_flag == 0 && STAR(S0_HEAD) == 0)
        STAR(S0_HEAD) = user + 256;
    if (plain_flag == 0 && STAR(R0_HEAD) == 0)
        STAR(R0_HEAD) = user + 512;
    if (plain_flag == 0 && STAR(TIB_HEAD) == 0)
        STAR(TIB_HEAD) = user + 256;
    //user = 0xf000;
    fprintf(stderr, "init_mem: up: %04x, s0: %04x, r0: %04x, tib: %04x\n", STAR(UP_HEAD), STAR(S0_HEAD), STAR(R0_HEAD), STAR(TIB_HEAD));
    // init user area
    src = &mem[DP_HEAD];
    dest = &mem[DP_ADDR];
    size = END_ADDR - DP_ADDR;
    memcpy(dest, src, size);
    fprintf(stderr, "init_mem: user copy: dest = %04x, src = %04x, size = %d\n", (unsigned int)(dest - mem), (unsigned int)(src - mem), size);

    // DICTTOP has already been set, no need to care here

    // halt addr is needed for 'execute'
    if (plain_flag == 0 && STAR(HALT_HEAD) == 0) {
        flag |= name2xt(cx, "halt");
        STAR(HALT_HEAD) = do_pop(cx);
    }
    fprintf(stderr, "init_mem: halt: %04x\n", STAR(HALT_HEAD));
    // cold vector, startup point if it is defined.
    //STAR(COLD_HEAD) = 0;
    //if (name2xt(cx, "cold") == 0)
    //    STAR(COLD_HEAD) = do_pop(cx);
    if (flag) {
        return -1;
    }
    //STAR(DEBUG_ADDR) = 1;
    if (STAR(DEBUG_ADDR)) {
        fprintf(stderr, "init_mem: initial DEBUG_ADDR = %d\n", STAR(DEBUG_ADDR));
    }
    // copy userhead to user area
    //fprintf(stderr, "init_mem: halt xt = %04X, semi xt = %04X\n", STAR(HALT_ADDR), STAR(SEMI_ADDR));
    return 0;
}

const char *output_file = NULL;
int disdump_flag = 0;

int main (int ac, char **av)
{
    // outer interpreter
    char buf[80];
    int n, result;
    word_t cold_addr, org;
    context_t _ctx, *cx;

    if (ac > 2 && strcmp(av[1], "-d") == 0) {
        // disdump dict.X
        disdump_flag = 1;
        av += 2;
        ac -= 2;
    }
    if (ac > 2 && strcmp(av[1], "-o") == 0) {
        output_file = av[2];
        av += 2;
        ac -= 2;
    }
    if (ac > 1 && strcmp(av[1], "--plain") == 0) {
        plain_flag = 1;
        av += 1;
        ac -= 1;
    }
    // init source file args
    // initialize ctx
    cx = &_ctx;
    initialize_ctx(cx);
    if ((org = read_dict(cx, av[1])) < 0) {
        fprintf(stderr, "exit read_dict error\n");
        return 1;
    }
    init_org(cx, org);
    init_outer_buf(ac - 2, av + 2);
    init_optable();
    if (init_mem(cx, org) < 0) {
        fprintf(stderr, "exit init_mem error\n");
        return 1;
    }
    cold_addr = STAR(COLD_HEAD);
    if (cold_addr) {
        fprintf(stderr, "start cold at %04x\n", cold_addr);
        do_push(cx, cold_addr);
        // STAR(DEBUG_ADDR) = 1;
        changemode(1);
        if ((result = setjmp(cx->env)) != 0) {
            if (result == -1) { // bye command, exit forth interpreter
                changemode(0);
                return 0;
            }
            fprintf(stderr, "longjmp: %d\n", result);
        } else {
            do_execute(cx);
        }
        changemode(0);
    } else {
        fprintf(stderr, "start text interpreter\n");
        //STAR(DEBUG_ADDR) = 1;
        // dump_hex(cx, org_addr, 54);
        // dump_hex(cx, STAR(UP_HEAD), 30);
        changemode(1);
        do_mainloop(cx);
        changemode(0);
    }
    return 0;
}

