#
#
#
.PREFIXES: .dict .list .f .X .s

NAME=cpnr

ALL=dict.X

all: cpnr dict.X

dict.X: dict.list
	sh dump.sh $< > $@
dict.list: dict.s
	sh as.sh $< > $@
dict.s: dict.dict opcode.inc makedict.sh
	sh makedict.sh $< > $@

opcode.inc: opcode.c makeopcode.sh
	sh makeopcode.sh $< > $@

opname.h: opcode.inc gen_opname.sh
	sh gen_opname.sh opcode.inc > $@

COBJS=main.o machine.o monitor.o cfunc.o opcode.o
#CFLAGS+=-g
CFLAGS+=-Wno-pointer-sign 

main.o machine.o monitor.o cfunc.o: machine.h
monitor.o: opname.h

cpnr: $(COBJS)
	cc -o cpnr $(COBJS)

.f.X:
	$(F2X) -o $*.X $*.f

f2x: f2x.c
	cc -o f2x f2x.c

clean:
	$(RM) $(COBJS) $(LISTS)
