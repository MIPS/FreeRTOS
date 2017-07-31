To build the demo:

Set MIPS_INST_ROOT to point to the mips tool chain e.g.
export MIPS_INST_ROOT=~/imgtec/mips-mti-elf/2015.06-05/bin

Add to the path so the tools can be found e.g.
export PATH=~/imgtec/mips-mti-elf/2015.06-05/bin/:$PATH

Then build the demo

cd freertos/FreeRTOS/Demo/lwIP_MIPS_GCC/mipsFPGA
ARCH=m14kc make clean

To build a mips32 freeRTOS library:-
ARCH=m14kc make mips32_freeRTOS

To build a mips32 lwip library:-
ARCH=m14kc make mips32_lwip

To build the application:-
ARCH=m14kc make
