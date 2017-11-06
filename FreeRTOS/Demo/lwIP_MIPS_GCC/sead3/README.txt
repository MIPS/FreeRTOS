To build the demo:

Set MIPS_ELF_ROOT to point to the mips tool chain e.g.
export MIPS_ELF_ROOT=~/imgtec/mips-mti-elf/2015.06-05/bin

Add to the path so the tools can be found e.g.
export PATH=$(MIPS_ELF_ROOT)/bin:$PATH

Then build the demo

cd freertos/FreeRTOS/Demo/lwIP_MIPS_GCC/sead3
make clean
make ARCH=m14kc (or whichever arch loaded into your FPGA)

To target a different toolchain triple (eg mips-img-elf) set CROSS_COMPILE e.g.
make ARCH=32r6 CROSS_COMPILE=mips-img-elf

