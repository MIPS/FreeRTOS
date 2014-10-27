To build the demo:

Set MIPS_INST_ROOT to point to the mips tool chain e.g.
export MIPS_INST_ROOT=~/imgtec/mips-mti-elf/2015.06-05/bin

Add to the path so the tools can be found e.g.
export PATH=~/imgtec/mips-mti-elf/2015.06-05/bin/:$PATH

Then build the demo.
cd freertos/FreeRTOS/Demo/lwIP_MIPS_GCC/CI40/

ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make clean

To build a mips32 freeRTOS library:-
ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make mips32_freeRTOS

To build a mips32 lwip library:-
ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make mips32_lwip


To build the application:-
ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make
