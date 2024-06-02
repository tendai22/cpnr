#
#
#
.PREFIXES: .dict .list .f .X .s

NAME=cpnr

ALL=dict.X

all: cpnr dict.X user.f inner.s

dict.X: dict.list
	sh dump.sh $< > $@
dict.list: dict.s
	sh as.sh dict.s > $@
primary.s: primary.dict opcode.inc makedict.sh
	sh makedict.sh $< > $@
dict.s: user.s inner.s primary.s
	cat user.s inner.s primary.s > dict.s

opcode.inc: opcode.c machine.h makeopcode.sh
	sh makeopcode.sh machine.h opcode.c > $@

opname.h: opcode.inc gen_opname.sh
	sh gen_opname.sh opcode.inc > $@

user.f user.h user.s: user.def makeuser.sh
	sh makeuser.sh -f user.def > user.f
	sh makeuser.sh -h user.def > user.h
	sh makeuser.sh -s user.def > user.s

COBJS=main.o machine.o monitor.o cfunc.o opcode.o
CFLAGS+=-g
CFLAGS+=-Wno-pointer-sign 

main.o machine.o monitor.o cfunc.o: machine.h user.h
monitor.o: opname.h

cpnr: $(COBJS)
	cc $(CFLAGS) -o cpnr $(COBJS)

.f.X:
	$(F2X) -o $*.X $*.f

f2x: f2x.c
	cc -o f2x f2x.c

clean:
	$(RM) $(COBJS) $(LISTS)
