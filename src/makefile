#
#
#
.PREFIXES: .dict .list .f .X .s

NAME=cpnr

ALL=dict.X

all: cpnr dict.X 

.list.X:
	sh dump.sh $< > $@
.s.list:
	sh as.sh $< > $@
.dict.list:
	sh makedict.sh

COBJS=main.o machine.o monitor.o cfunc.o

main.o machine.o monitor.o cfunc.o: machine.h

cpnr: $(COBJS)
	cc -o cpnr $(COBJS)

dict.list: $(AOBJS)
	sh as.sh $(AOBJS) > dict.list

.f.X:
	$(F2X) -o $*.X $*.f

f2x: f2x.c
	cc -o f2x f2x.c

clean:
	$(RM) $(COBJS) $(LISTS)
