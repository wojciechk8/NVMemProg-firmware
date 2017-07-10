# The name for the project
#TARGET:=NVMemProg

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

# Memory interface modules
IFCMODDIR:=ifc_mod
IFCMODSRC:=$(wildcard $(IFCMODDIR)/*.c)
IFCMODTARGETS:=$(notdir $(basename $(IFCMODSRC)))

# Other include directories
INCDIR:=.

# Fx2lib directory
FX2LIBDIR:=/opt/fx2lib

# Target objects directory
OBJDIR:=obj
BINDIR:=bin

# Target interface for programming (set it from command line)
# ie. make program IFC=dummy
IFC?=dummy



# Internal variables names
OBJ:=$(SRC:%.c=$(OBJDIR)/%.rel) $(ASRC:%.a51=$(OBJDIR)/%.rel)
LST:=$(SRC:%.c=$(OBJDIR)/%.lst) $(ASRC:%.a51=$(OBJDIR)/%.lst)
RST:=$(SRC:%.c=$(OBJDIR)/%.rst) $(ASRC:%.a51=$(OBJDIR)/%.rst)
SYM:=$(SRC:%.c=$(OBJDIR)/%.sym) $(ASRC:%.a51=$(OBJDIR)/%.sym)
ASM:=$(SRC:%.c=$(OBJDIR)/%.asm) 
IHX:=$(addprefix $(BINDIR)/,$(addsuffix .ihx,$(IFCMODTARGETS)))
BIX:=$(addprefix $(BINDIR)/,$(addsuffix .bix,$(IFCMODTARGETS)))
IIC:=$(addprefix $(BINDIR)/,$(addsuffix .iic,$(IFCMODTARGETS)))
LK:=$(addprefix $(BINDIR)/,$(addsuffix .lk,$(IFCMODTARGETS)))
MEM:=$(addprefix $(BINDIR)/,$(addsuffix .mem,$(IFCMODTARGETS)))
MAP:=$(addprefix $(BINDIR)/,$(addsuffix .map,$(IFCMODTARGETS)))

IFCMODOBJ:=$(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.rel)))
IFCMODLST:=$(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.lst)))
IFCMODRST:=$(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.rst)))
IFCMODSYM:=$(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.sym)))
IFCMODASM:=$(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.asm))) 

# Dependecies
DEPENDS:=$(SRC:%.c=$(OBJDIR)/%.d) $(addprefix $(IFCMODDIR)/$(OBJDIR)/,$(notdir $(IFCMODSRC:%.c=%.d)))

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
all: $(BINDIR) $(OBJDIR) $(IFCMODDIR)/$(OBJDIR) $(IHX)
#	$(foreach f,$(IHX),$(SIZE) $(f))
	tail -n 5 $(MEM)

.PHONY: ihx
ihx: $(IHX)

.PHONY: bix
bix: $(BIX)

.PHONY: iic
iic: $(IIC)

.PHONY: program
program: $(IHX)
	$(FX2PROG) -id=$(VID).$(PID) prg:$(BINDIR)/$(IFC).ihx
	$(FX2PROG) -id=$(VID).$(PID) run
	$(SIZE) $(IHX)

.PHONY: run
run: $(IHX)
	$(FX2PROG) -id=$(VID).$(PID) reset
	$(FX2PROG) -id=$(VID).$(PID) run

.PHONY: clean
clean:
	$(RM) $(IHX) $(BIX) $(IIC) $(MEM) $(MAP) $(LK) \
$(OBJ) $(LST) $(RST) $(SYM) $(ASM) $(DEPENDS) \
$(IFCMODOBJ) $(IFCMODLST) $(IFCMODRST) $(IFCMODSYM) $(IFCMODASM)
	$(RM) -d $(BINDIR) $(OBJDIR) $(IFCMODDIR)/$(OBJDIR)


# implicit rules
$(OBJDIR)/%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MM $(CFLAGS) -c $< -o $(patsubst %.rel,%.d,$@)
	sed -i "1s/^/$(OBJDIR)\//" $(patsubst %.rel,%.d,$@)

$(OBJDIR)/%.rel: %.a51
	$(AS) -logs $(ASFLAGS) $@ $^

$(IFCMODDIR)/$(OBJDIR)/%.rel: $(IFCMODDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MM $(CFLAGS) -c $< -o $(patsubst %.rel,%.d,$@)
	sed -i "1s/^/$(IFCMODDIR)\/$(OBJDIR)\//" $(patsubst %.rel,%.d,$@)

$(BINDIR)/%.ihx: $(IFCMODDIR)/$(OBJDIR)/%.rel $(OBJ)
	$(CC) $(LDFLAGS) $(LDLIBS) $(OBJ) $< -o $@
	$(RM) $(MAP) $(LK)

$(BINDIR)/%.bix: $(BINDIR)/%.ihx
	$(OBJCOPY) -I ihex -O binary $< $@

$(BINDIR)/%.iic: $(BINDIR)/%.ihx
	$(FX2LIBDIR)/utils/ihx2iic.py -v $(VID) -p $(PID) $< $@

# explicit rules
$(OBJDIR):
	mkdir -p $(OBJDIR)
$(IFCMODDIR)/$(OBJDIR):
	mkdir -p $(IFCMODDIR)/$(OBJDIR)
$(BINDIR):
	mkdir -p $(BINDIR)

# include dependecies
-include $(DEPENDS)
