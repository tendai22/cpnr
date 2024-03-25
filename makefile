#
#
#
.PREFIXES: .f .X
NAME=cpnr

OBJS=codes.o dict.o
LISTS=$(NAME).dict $(NAME).list $(NAME).X
TMPS=dict.s a.dict a.out a.symbols codes.list dict.list

.f.X:
	$(F2X) -o $*.X $*.f

all: $(NAME).X bp.X $(NAME).dict $(NAME).list forth.X $(F2X)

a.out:  $(OBJS)
	$(LD) -T trip.ldscript $(OBJS)

$(NAME).list:  a.out
	$(OBJDUMP) --start-address=0x1000 --stop-address=0x1fff -D a.out |tee $(NAME).list

$(NAME).dict: a.out dictdump
	sh dictdump.sh > $(NAME).dict 

$(NAME).X: a.out
	sh dump.sh > $(NAME).X

f2x: f2x.c
	cc -o f2x f2x.c

bp.X: a.out
	sh extract_bp.sh > bp.X

forth.X: forth.f
	./f2x -o forth.X forth.f

dict.s:  base.dict makedict.sh
	sh makedict.sh > dict.s

dictdump: dictdump.c
	cc -o dictdump dictdump.c

clean:
	$(RM) $(OBJS) $(LISTS)
