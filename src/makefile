#
#
#
.PREFIXES: .dict .list .f .X .s

NAME=cpnr

ALL=dict.X

all: cpnr dict.list 

dict.X: dict.list
	sh dump.sh $< > $@
dict.list: dict.s
	sh as.sh $< > $@
dict.s: dict.dict opcode.inc makedict.sh
	sh makedict.sh $< > $@

opcode.inc: opcode.c makeopcode.sh
	sh makeopcode.sh $< > $@

COBJS=main.o machine.o monitor.o cfunc.o opcode.o

main.o machine.o monitor.o cfunc.o: machine.h

cpnr: $(COBJS)
	cc -o cpnr $(COBJS)

.f.X:
	$(F2X) -o $*.X $*.f

f2x: f2x.c
	cc -o f2x f2x.c

clean:
	$(RM) $(COBJS) $(LISTS)
