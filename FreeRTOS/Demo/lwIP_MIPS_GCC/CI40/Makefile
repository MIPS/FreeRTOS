# Makefile for FreeRTOS demo
#

TOP = $(PWD)

# Are we compiling for the Rev 6 ISA
ARCH ?=mips32r5
ifeq ($(ARCH),mips32r6)
CROSS_COMPILE ?= mips-img-elf
else
CROSS_COMPILE ?= mips-mti-elf
endif
GCC = $(CROSS_COMPILE)-gcc

# Are we using microMIPS
ifeq ($(MICROMIPS),1)
UMIPS_FLAGS := -mmicromips -minterlink-compressed
endif

ifeq ($(MICROMIPS_EXCEPTIONS),1)
DEFINES += -DMICROMIPS_EXCEPTIONS
UMIPS_FLAGS += -minterlink-compressed
endif

# Are we compiling in DSP support
ifeq ($(DSP),1)
DEFINES += -DWITH_DSP
DSP_FLAGS := -mdspr2
endif

DEFINES += -DWITH_GIC_VI -DLITTLE_ENDIAN=1


DEFINES += -DGIC_BASE_ADDRESS=0x1BDC0000 -DconfigCPU_CLOCK_HZ=400000000


ifdef DEMO_DEBUG
DEFINES += -DDEMO_DEBUG
endif


# Select the floating point mechanism
FLOATS ?=-msoft-float

# Include paths
INCLUDE = -I. -I.. -I../include -I../../../Source/include -I../../../Source/portable/GCC/MIPS32 -I../../Common/include -I../lwip/src/include 
INCLUDE += -I./include -I../lwip/src/include/ipv4
INCLUDE += -I../../../Source/portable/GCC/MIPS32/ -I../common/ -I ../common/include

# Which interrupt mechanism? CI40 is VI by default.
INTS ?= vi


ENDIAN ?= -EL

# C Flags
CFLAGS = -march=$(ARCH) $(ENDIAN) -g $(FLOATS) $(UMIPS_FLAGS) $(DSP_FLAGS) $(DEFINES) $(INCLUDE)

# Source & object files
FR_SRC := \
	../../../Source/queue.c \
	../../../Source/tasks.c \
	../../../Source/list.c \
	../../../Source/timers.c \
	../../../Source/portable/GCC/MIPS32/port.c \
	../../../Source/portable/GCC/MIPS32/$(INTS)/int_handler.c \
	../../../Source/portable/GCC/MIPS32/$(INTS)/port_timer.c \
	../../../Source/portable/GCC/MIPS32/$(INTS)/init_hook.c \
	../../../Source/portable/MemMang/heap_1.c \
	../../../Source/event_groups.c

LWIP_SRC := \
	../lwip/src/core/tcp_out.c \
	../lwip/src/core/inet.c \
	../lwip/src/core/mem.c \
	../lwip/src/core/memp.c \
	../lwip/src/core/netif.c \
	../lwip/src/core/pbuf.c \
	../lwip/src/core/raw.c \
	../lwip/src/core/stats.c \
	../lwip/src/core/sys.c \
	../lwip/src/core/tcp.c \
	../lwip/src/core/tcp_in.c \
	../lwip/src/core/lwip_timers.c \
	../lwip/src/core/udp.c \
	../lwip/src/core/init.c \
	../lwip/src/core/dhcp.c \
	../lwip/src/core/dns.c \
	../lwip/src/core/ipv4/ip.c \
	../lwip/src/core/ipv4/ip_addr.c \
	../lwip/src/core/ipv4/icmp.c \
	../lwip/src/core/ipv4/ip_frag.c \
	../lwip/src/core/ipv4/def.c \
	../lwip/src/core/ipv4/inet_chksum.c \
	../lwip/src/api/tcpip.c \
	../lwip/src/api/api_msg.c \
	../lwip/src/api/err.c \
	../lwip/src/api/api_lib.c \
	../lwip/src/api/sockets.c \
	../lwip/src/api/netbuf.c \
	../lwip/src/netif/etharp.c \
	../lwip/sys_arch.c

APP_SRC := \
	../common/web.c \
	../common/uart16550.c \
	../common/vi_intc.c \
	ci40_net.c \
	ci40_eth.c \
	main.c
	
ifeq ($(TRACE),1)
DEFINES +=-DENABLE_TRACE
endif

ASMSRC := \
	../../../Source/portable/GCC/MIPS32/port_asm.S \
	../../../Source/portable/GCC/MIPS32/$(INTS)/handler.S \
	../common/vi_isr.S

OBJDIR ?= build
FR_TMPOBJ := $(FR_SRC:.c=.o) $(ASMSRC:.S=.o)
LWIP_TMPOBJ := $(LWIP_SRC:.c=.o) $(ASMSRC:.S=.o)
APP_TMPOBJ := $(APP_SRC:.c=.o) $(ASMSRC:.S=.o)

FR_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(FR_TMPOBJ)))
LWIP_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(LWIP_TMPOBJ)))
APP_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(APP_TMPOBJ)))

# Rules
FR_LIBNAME = mips32_freeRTOS
LWIP_LIBNAME = mips32_lwip
FR_TARGET = lib$(FR_LIBNAME).a
LWIP_TARGET = lib$(LWIP_LIBNAME).a
APP_TARGET = RTOSDemo.elf

APP_LIBS := -L../ -Wl,-l$(FR_LIBNAME) -Wl,-l$(LWIP_LIBNAME)

.PHONY: all clean $(FR_LIBNAME) $(LWIP_LIBNAME)
all: $(FR_TARGET) $(LWIP_TARGET) $(APP_TARGET)

$(FR_LIBNAME): $(FR_TARGET)
$(LWIP_LIBNAME): $(LWIP_TARGET)

$(FR_TARGET): $(FR_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(LWIP_TARGET): $(LWIP_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(APP_TARGET): $(APP_OBJ)
	$(GCC) $(CFLAGS) -Wl,-Map=$(subst .elf,.map,$@) -T./uhi32.ld -o $@ $^ -Wl,-lm $(APP_LIBS)

OBJ := $(FR_OBJ) $(LWIP_OBJ) $(APP_OBJ)

$(OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o : ../../Common/Minimal/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../../../Source/portable/MemMang/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../../../Source/portable/GCC/MIPS32/%.c
	$(GCC) -c $(CFLAGS) $< -o $@
	
$(OBJDIR)/%.o : ../../../Source/portable/GCC/MIPS32/$(INTS)/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../../../Source/%.c
	$(GCC) -c $(CFLAGS) $< -o $@
	
$(OBJDIR)/%.o : %.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../../../Source/portable/GCC/MIPS32/%.S
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../../../Source/portable/GCC/MIPS32/%.S
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/src/core/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/src/core/ipv4/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/src/api/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/ports/Ci40/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../lwip/src/netif/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../common/%.c
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : ../common/%.S
	$(GCC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.S
	$(GCC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf build
	rm -f $(FR_TARGET) $(subst .a,.map,$(FR_TARGET))
	rm -f $(LWIP_TARGET) $(subst .a,.map,$(LWIP_TARGET))
	rm -f $(APP_TARGET) $(subst .elf,.map,$(APP_TARGET))
