#
# TARGET dict build
#
.PREFIXES: .dict .list .f .X .s

TARGET=nr1
TARGET_ORG=0x1000
TARGET_USER=0xe000
HOST_ORG=0x8000
HOST_USER=0xf000

TARGET_INTERFILES=$(TARGET).bin $(TARGET)_user.f $(TARGET)_user.s $(TARGET)_org.f \
	$(TARGET)_dict.f $(TARGET)_dict.X $(TARGET)_dict.list $(TARGET)_dict.s
$(TARGET).bin: cpnr cross8.bin $(TARGET)_dict.f $(TARGET)_org.f start_cross.f $(TARGET)_user.f base.f cold.f end_cross.f dictdump.f
	./cpnr -o $@ cross8.bin $(TARGET)_dict.f $(TARGET)_org.f start_cross.f $(TARGET)_user.f base.f cold2.f end_cross.f dictdump.f
$(TARGET)_user.f: user.def makeuser.sh
	sh makeuser.sh -f $(TARGET_ORG) $(TARGET_USER) $< > $@
$(TARGET)_dict.f: $(TARGET)_dict.X
	sh X2f.sh $< > $@
$(TARGET)_dict.X: $(TARGET)_dict.list
	sh dump.sh $< > $@
$(TARGET)_dict.list: $(TARGET)_dict.s
	sh as.sh $< > $@
$(TARGET)_dict.s: $(TARGET)_user.s $(TARGET)_inner.s $(TARGET)_primary.s
	cat $(TARGET)_user.s $(TARGET)_inner.s $(TARGET)_primary.s > $@
$(TARGET)_user.s: user.def makeuser.sh
	sh makeuser.sh -s $(TARGET_ORG) $(TARGET_USER) $< > $@
$(TARGET)_org.f: makefile
	echo $(TARGET_ORG) $(TARGET_USER) > $(TARGET)_org.f

cross8.bin: cpnr dict.X user.f base.f dictdump.f
	./cpnr -o $@ dict.X user.f base.f cold.f dictdump.f

NAME=cpnr

ALL=dict.X

all: cpnr dict.X user.f inner.s

INTERFILES=dict.X user.f user.s user.h opcode.inc dict.s primary.s dict.list

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

user.f user.s: user.def makeuser.sh
	sh makeuser.sh -f $(HOST_ORG) $(HOST_USER) user.def > user.f
	sh makeuser.sh -s $(HOST_ORG) $(HOST_USER) user.def > user.s

#
# cpnr ... C language program
#
COBJS=main.o machine.o monitor.o cfunc.o opcode.o key_linux.o
CFLAGS+=-g
CFLAGS+=-Wno-pointer-sign 

cfunc.o machine.o main.o monitor.o opcode.o: machine.h user.h
cfunc.o key_linux.o main.o opcode.o: key_in.h
monitor.o: opname.h

opname.h: opcode.inc gen_opname.sh
	sh gen_opname.sh opcode.inc > $@

cpnr: $(COBJS)
	cc $(CFLAGS) -o cpnr $(COBJS)

user.h: user.def makeuser.sh
	sh makeuser.sh -h $(HOST_ORG) $(HOST_USER) user.def > user.h

.f.X:
	$(F2X) -o $*.X $*.f

f2x: f2x.c
	cc -o f2x f2x.c

clean:
	$(RM) $(COBJS) $(LISTS) $(INTERFILES) $(TARGET_INTERFILES)