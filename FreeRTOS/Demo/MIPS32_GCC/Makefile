# Makefile for FreeRTOS demo
#
CONFIG_FILE=.config
CONFIGS_DIR=configs
$(shell if [ ! -e $(CONFIG_FILE) ]; then \
		touch $(CONFIG_FILE); \
		make defaultconfig > $(CONFIG_FILE); \
		fi)
include $(CONFIG_FILE)

TOP = $(PWD)

# Are we compiling for the Rev 6 ISA
ifeq ($(ARCH),mips32r6)
CROSS_COMPILE ?= mips-img-elf
else
CROSS_COMPILE ?= mips-mti-elf
endif
GCC = $(CROSS_COMPILE)-gcc

# Big Endian instead of LE?
ifeq ($(BE),1)
CFLAGS += -EB
else
CFLAGS += -EL
endif

# Are we using microMIPS
ifeq ($(UMIPS),1)
CFLAGS += -mmicromips -minterlink-compressed
endif

# Are we compiling in DSP support
ifeq ($(DSP),1)
CFLAGS += -mdspr2
endif

# Are we compiling in hardware floating point support
ifeq ($(FPU),1)
CFLAGS += -mhard-float
else
CFLAGS += -msoft-float
endif

# by default we try to enable GIC EIC if possible, if you want to force it to be
# off set NO_GIC_EIC=1
ifeq ($(NO_GIC_EIC),1)
CFLAGS += -DCONFIG_NO_GIC_EIC
endif

# if GIC is detected, we reset external interrupts setup by default. if you want
# to inherit the settings from an external (possibly bootloader) source, set
# INHERIT_GIC=1
ifeq ($(INHERIT_GIC),1)
CFLAGS += -DCONFIG_INHERIT_GIC
endif

# Needs retesting and verification
ifeq ($(TRACE),1)
CFLAGS +=-DENABLE_TRACE
endif

MIPS32_SRC_DIR=../../Source/portable/GCC/MIPS32
MIPS_ELF_ROOT ?= $(shell dirname `locate -n 1 $(GCC)`)

# Include paths
INCLUDE = -I. -I./include -I../../Source/include -I$(MIPS32_SRC_DIR) -I../Common/include

# C Flags
CFLAGS += -march=$(ARCH) -g $(INCLUDE)

# Customise FreeRTOS settings
CFLAGS += -DCONFIG_CPU_CLOCK_HZ=$(CPU_CLOCK_HZ)

CFLAGS += -DCONFIG_TICK_RATE_HZ=$(TICK_RATE_HZ)
CFLAGS += -DCONFIG_TIMER_CLOCK_HZ=$(TIMER_CLOCK_HZ)


CFLAGS += -DCONFIG_STACK_SIZE=$(STACK_SIZE)
CFLAGS += -DCONFIG_HEAP_SIZE=$(HEAP_SIZE)
CFLAGS += -DCONFIG_MAX_PRIORITIES=$(MAX_PRIORITIES)

# Use default uhi32.ld from SDK. If we use bootable code use our own which
# contains some modifications to support moving the boot_start. Hopefully in
# the future the SDK will support that and we can use the default one from the
# SDK too.
ifeq ($(ROMABLE), 0)
LDSCRIPT=-Tuhi32.ld
else
ifeq ($(TWO_STAGE_BOOT), 0)
LDSCRIPT=-Tbootcode.ld -Tboot/uhi32.ld
LDSCRIPT+=-Wl,--defsym,__bootrom_start=$(BOOT_START)
else
LDSCRIPT=-Tuhi32.ld
BOOT_LDSCRIPT=-Tbootcode.ld -Tboot/uhi32.ld
BOOT_LDSCRIPT+=-Wl,--defsym,__bootrom_start=$(BOOT_START)
BOOT_LDSCRIPT+=-Wl,--defsym,__app_start=$(BOOT_START)
BOOT_LDSCRIPT+=-Wl,--defsym,__memory_size=32K
BOOT_LDSCRIPT+=-Wl,--defsym,__memory_base=$(BOOT_START)
endif
endif

# Set vector count to maximum possible (64) when eic is selected.
# This avoids the potential problem of overwriting text section when installing
# an isr due to overlapping.
LDSCRIPT+=-Wl,--defsym,__isr_vec_count=64

# __use_excpt_boot means we can use a bootloader (ie: uboot) to perform uhi
# functionality. We can't assume such a bootloader exist so always set it to 0.
LDSCRIPT+=-Wl,--defsym,__use_excpt_boot=0

# some sensible defaults for memory setup and application start
LDSCRIPT+=-Wl,--defsym,__app_start=$(APP_START)
LDSCRIPT+=-Wl,--defsym,__memory_size=$(MEMORY_SIZE)
LDSCRIPT+=-Wl,--defsym,__memory_base=$(MEMORY_BASE)

ifeq ($(ROMABLE), 1)
CFLAGS += -DC0_CONFIG0_VALUE=$(C0_CONFIG0_VALUE)
CFLAGS += -DC0_CONFIG1_VALUE=$(C0_CONFIG1_VALUE)
CFLAGS += -DC0_CONFIG2_VALUE=$(C0_CONFIG2_VALUE)
CFLAGS += -DC0_CONFIG3_VALUE=$(C0_CONFIG3_VALUE)
CFLAGS += -DC0_CONFIG4_VALUE=$(C0_CONFIG4_VALUE)
CFLAGS += -DC0_CONFIG5_VALUE=$(C0_CONFIG5_VALUE)
CFLAGS += -DC0_WATCHHI_VALUE=$(C0_WATCHHI_VALUE)
CFLAGS += -DC0_WATCHHI1_VALUE=$(C0_WATCHHI1_VALUE)
CFLAGS += -DC0_WATCHHI2_VALUE=$(C0_WATCHHI2_VALUE)
CFLAGS += -DC0_WATCHHI3_VALUE=$(C0_WATCHHI3_VALUE)
CFLAGS += -DC0_WATCHHI4_VALUE=$(C0_WATCHHI4_VALUE)
CFLAGS += -DC0_WATCHHI5_VALUE=$(C0_WATCHHI5_VALUE)
CFLAGS += -DC0_WATCHHI6_VALUE=$(C0_WATCHHI6_VALUE)
CFLAGS += -DC0_WATCHHI7_VALUE=$(C0_WATCHHI7_VALUE)
endif

# Source & object files
SRC := \
	../../Source/queue.c \
	../../Source/tasks.c \
	../../Source/list.c \
	../../Source/timers.c \
	$(MIPS32_SRC_DIR)/port.c \
	$(MIPS32_SRC_DIR)/int_handler.c \
	$(MIPS32_SRC_DIR)/port_timer.c \
	$(MIPS32_SRC_DIR)/init_hook.c \
	../../Source/portable/MemMang/heap_1.c \
	../../Source/event_groups.c \
	main.c

ASMSRC := \
	$(MIPS32_SRC_DIR)/port_asm.S \
	$(MIPS32_SRC_DIR)/handler.S

ifeq ($(ROMABLE), 1)
INCLUDE += -I $(MIPS_ELF_ROOT)/share/mips/boot
BOOTASMSRC := \
	boot/reset_predef.S \
	$(MIPS_ELF_ROOT)/share/mips/boot/init_caches_predef.S \
	$(MIPS_ELF_ROOT)/share/mips/boot/init_l23caches_predef.S \
	$(MIPS_ELF_ROOT)/share/mips/boot/init_cp0_predef.S \
	$(MIPS_ELF_ROOT)/share/mips/boot/init_tlb_predef.S \
	$(MIPS_ELF_ROOT)/share/mips/boot/corecheck_predef.S

BOOTSRC := \
	boot/boot_hw_init.c
endif

OBJDIR ?= build

BOBJ := $(BOOTSRC:.c=.o) $(BOOTASMSRC:.S=.o)
TOBJ := $(SRC:.c=.o) $(ASMSRC:.S=.o)

ifeq ($(TWO_STAGE_BOOT), 0)
TOBJ += $(BOBJ)
endif

TARGET_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(TOBJ)))
BOOT_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(BOBJ)))

# Rules
TARGET = RTOSDemo.elf

ifeq ($(TWO_STAGE_BOOT), 1)
TARGET_BOOT = boot.elf
CFLAGS += -DTWO_STAGE_BOOT
endif

CONFIGS=$(shell cd $(CONFIGS_DIR); ls *)

.PHONY: all clean $(CONFIGS)
all: $(TARGET) $(TARGET_BOOT)

$(TARGET): $(TARGET_OBJ)
	$(GCC) $(CFLAGS) -Wl,-Map=$(subst .elf,.map,$@) $(LDSCRIPT) -o $@ $^

ifeq ($(TWO_STAGE_BOOT), 1)
$(TARGET_BOOT): $(BOOT_OBJ)
	$(GCC) $(CFLAGS) -Wl,-Map=$(subst .elf,.map,$@) $(BOOT_LDSCRIPT) -o $@ $^
endif

$(TARGET_OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o : ../Common/Minimal/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : ../../Source/portable/MemMang/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : $(MIPS32_SRC_DIR)/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : ../../Source/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : %.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : $(MIPS32_SRC_DIR)/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : boot/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : boot/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : %.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o: $(MIPS_ELF_ROOT)/share/mips/boot/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

# We need to modify reset_predef.S to support booting in EVA mode but we don't
# want to make the source file part of our source as it has different copyright
# header. Copy it at runtime and patch it to avoid redistributing it.
boot/reset_predef.S: $(MIPS_ELF_ROOT)/share/mips/boot/reset_predef.S
	@cp $^ $@
	@patch -p0 < boot/reset_predef.patch

clean:
	rm -rf build
	rm -f $(TARGET) $(subst .elf,.map,$(TARGET))
ifeq ($(TWO_STAGE_BOOT), 1)
	rm -f $(TARGET_BOOT) $(subst .elf,.map,$(TARGET_BOOT))
endif

$(CONFIGS):
	@cat $(CONFIGS_DIR)/$@ > $(CONFIG_FILE)

-include $(TARGET_OBJ:.o=.d)
-include $(BOOT_OBJ:.o=.d)
