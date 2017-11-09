# The name for the project
#TARGET:=NVMemProg

# Device VID, PID
VID:=0x1209
PID:=0x4801

# Target interface module for programming (set it from command line)
# ie. make program IFC=dummy
IFC?=dummy

# Driver name
DRIVER:=default

# Linker parameters
DSCR_AREA:=-Wl"-b DSCR_AREA=0x3e00"
INT2JT:=-Wl"-b INT2JT=0x3f00"
CODE_SIZE:=--code-size 0x3c00
XRAM_SIZE:=--xram-size 0x0200
XRAM_LOC:=--xram-loc 0x3c00

# The source files of the project
SRC:=delay_us.c driver.c fpga.c nvmemprog.c power.c
ASRC:=dscr.a51

# Memory interface modules
IFCMODDIR:=ifc_mod
IFCMODSRC:=dummy.c mx28f.c 29lv.c mbm27c.c 27c512.c 28f.c

# Other include directories
INCDIR:=.

# Fx2lib directory
FX2LIBDIR:=/opt/fx2lib

# Target objects directory
OBJDIR:=obj
BINDIR:=bin



# Internal variables

# Interface module targets
IFCMODTARGETS:=$(IFCMODSRC)
IFCMODSRC:=$(addprefix $(IFCMODDIR)/,$(IFCMODSRC))
IFCMODOBJ:=$(IFCMODTARGETS:%.c=$(IFCMODDIR)/$(OBJDIR)/%.rel)
IFCMODLST:=$(IFCMODOBJ:%.rel=%.lst)
IFCMODRST:=$(IFCMODOBJ:%.rel=%.rst)
IFCMODSYM:=$(IFCMODOBJ:%.rel=%.sym)
IFCMODASM:=$(IFCMODOBJ:%.rel=%.asm)

# Main targets
SRC+= driver_$(DRIVER).c
OBJ:=$(SRC:%.c=$(OBJDIR)/%.rel) $(ASRC:%.a51=$(OBJDIR)/%.rel)
LST:=$(OBJ:%.rel=%.lst)
RST:=$(OBJ:%.rel=%.rst)
SYM:=$(OBJ:%.rel=%.sym)
ASM:=$(OBJ:%.rel=%.asm)
IHX:=$(addprefix $(BINDIR)/,$(IFCMODTARGETS:%.c=%.ihx))
BIX:=$(IHX:%.ihx=%.bix)
IIC:=$(IHX:%.ihx=%.iic)
LK:=$(IHX:%.ihx=%.lk)
MEM:=$(IHX:%.ihx=%.mem)
MAP:=$(IHX:%.ihx=%.map)

# Dependecies
DEPENDS:=$(SRC:%.c=$(OBJDIR)/%.d) $(ASRC:%.a51=$(OBJDIR)/%.d) $(IFCMODOBJ:%.rel=%.d)

# Build tools
CC:=sdcc
AS:=sdas8051
OBJCOPY:=objcopy
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


# Targets
.PHONY: all
all: dirs $(IHX)
	@tail -n 5 $(MEM)

.PHONY: dirs
dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(IFCMODDIR)/$(OBJDIR)
	@mkdir -p $(BINDIR)

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
	tail -n 5 $(MEM)

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


# build rules
.SECONDARY: $(OBJ) $(IFCMODOBJ)

$(BINDIR)/%.ihx: $(IFCMODDIR)/$(OBJDIR)/%.rel $(OBJ)
	$(CC) $(LDFLAGS) $(LDLIBS) $(OBJ) $< -o $@
	$(RM) $(MAP) $(LK)

$(BINDIR)/%.bix: $(BINDIR)/%.ihx
	$(OBJCOPY) -I ihex -O binary $< $@

$(BINDIR)/%.iic: $(BINDIR)/%.ihx
	$(FX2LIBDIR)/utils/ihx2iic.py -v $(VID) -p $(PID) $< $@

$(OBJDIR)/%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) -c $< -o $(patsubst %.rel,%.d,$@)
	@sed -i "1s/^/$(OBJDIR)\//" $(patsubst %.rel,%.d,$@)

$(OBJDIR)/%.rel: %.a51
	$(AS) -logs $(ASFLAGS) $@ $^

$(IFCMODDIR)/$(OBJDIR)/%.rel: $(IFCMODDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) -c $< -o $(patsubst %.rel,%.d,$@)
	@sed -i "1s/^/$(IFCMODDIR)\/$(OBJDIR)\//" $(patsubst %.rel,%.d,$@)

# include dependecies
-include $(DEPENDS)
