# The name for the project
TARGET:=NVMemProg

# Device VID, PID
VID:=0x04b4
PID:=0x8613

# Linker parameters
DSCR_AREA:=-Wl"-b DSCR_AREA=0x3e00"
INT2JT:=-Wl"-b INT2JT=0x3f00"
CODE_SIZE:=--code-size 0x3c00
XRAM_SIZE:=--xram-size 0x0200
XRAM_LOC:=--xram-loc 0x3c00

# The source files of the project
SRC:=$(wildcard *.c)
ASRC:=$(wildcard *.a51)

# Other include directories
INCDIR:=.

# Fx2lib directory
FX2LIBDIR:=/opt/fx2lib

# Target objects directory
OBJDIR:=obj
BINDIR:=bin

# Enable debug output
DEBUG:=--debug



# Internal variables names
OBJ:=$(SRC:%.c=$(OBJDIR)/%.rel) $(ASRC:%.a51=$(OBJDIR)/%.rel)
LST:=$(SRC:%.c=$(OBJDIR)/%.lst) $(ASRC:%.a51=$(OBJDIR)/%.lst)
RST:=$(SRC:%.c=$(OBJDIR)/%.rst) $(ASRC:%.a51=$(OBJDIR)/%.rst)
SYM:=$(SRC:%.c=$(OBJDIR)/%.sym) $(ASRC:%.a51=$(OBJDIR)/%.sym)
ADB:=$(SRC:%.c=$(OBJDIR)/%.adb)
ASM:=$(SRC:%.c=$(OBJDIR)/%.asm)
IHX:=$(BINDIR)/$(TARGET).ihx
BIX:=$(BINDIR)/$(TARGET).bix
IIC:=$(BINDIR)/$(TARGET).iic
CDB:=$(BINDIR)/$(TARGET).cdb
OMF:=$(BINDIR)/$(TARGET).omf
LK:=$(BINDIR)/$(TARGET).lk
MEM:=$(BINDIR)/$(TARGET).mem
MAP:=$(BINDIR)/$(TARGET).map
# Dependecies
DEPENDS:=$(SRC:%.c=$(OBJDIR)/%.d)

# Build tools
CC:=sdcc
AS:=sdas8051
OBJCOPY:=objcopy
OBJDUMP:=objdump
SIZE:=size

FX2PROG:=cycfx2prog

# Common flags
COMMON_FLAGS:=-mmcs51
# Assembler flags
ASFLAGS:=
# C flags
CFLAGS:=$(COMMON_FLAGS) --std-sdcc99
CFLAGS+= -I$(FX2LIBDIR)/include -I$(INCDIR)
# LD flags
LDFLAGS:=$(COMMON_FLAGS) -L $(FX2LIBDIR)/lib
LDFLAGS+= $(CODE_SIZE) $(XRAM_SIZE) $(XRAM_LOC) $(DSCR_AREA) $(INT2JT)
LDLIBS:=fx2.lib


# Additional Suffixes
#.SUFFIXES: .rel .ihx .bix

# Targets
.PHONY: all
all: $(IHX)
#	 $(SIZE) $(IHX)
	 cat $(MEM)

.PHONY: ihx
ihx: $(IHX)

.PHONY: bix
bix: $(BIX)

.PHONY: iic
iic: $(IIC)

.PHONY: program
program: $(IHX)
	$(FX2PROG) -id=$(VID).$(PID) prg:$(IHX)
	$(FX2PROG) -id=$(VID).$(PID) run
	$(SIZE) $(IHX)

.PHONY: run
run: $(IHX)
	$(FX2PROG) -id=$(VID).$(PID) reset
	$(FX2PROG) -id=$(VID).$(PID) run

.PHONY: clean
clean:
	$(RM) $(IHX) $(BIX) $(IIC) $(MEM) $(MAP) $(LK) $(CDB) $(OMF) \
$(OBJ) $(LST) $(RST) $(SYM) $(ADB) $(ASM) $(DEPENDS)

# implicit rules
$(OBJDIR)/%.rel: %.c
	$(CC) $(DEBUG) $(CFLAGS) -c $< -o $@
	$(CC) -MM $(CFLAGS) -c $< -o $(patsubst %.rel,%.d,$@)
	sed -i "1s/^/$(OBJDIR)\//" $(patsubst %.rel,%.d,$@)

$(OBJDIR)/%.rel: %.a51
	$(AS) -logs $(ASFLAGS) $@ $^

# explicit rules
$(OBJDIR):
	mkdir -p $(OBJDIR)
$(BINDIR):
	mkdir -p $(BINDIR)
		
$(IHX): $(BINDIR) $(OBJDIR) $(OBJ)
	$(CC) $(DEBUG) $(LDFLAGS) $(LDLIBS) $(OBJ) -o $@

#$(DIS): $(OMF)
#	$(OBJDUMP) -S $< > $@

$(BIX): $(IHX)
	$(OBJCOPY) -I ihex -O binary $< $@
	
$(IIC): $(IHX)
	$(FX2LIBDIR)/utils/ihx2iic.py -v $(VID) -p $(PID) $< $@

# include dependecies
-include $(DEPENDS)
