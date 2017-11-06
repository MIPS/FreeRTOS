To build the demo:

Set MIPS_ELF_ROOT to point to the mips tool chain e.g.
export MIPS_ELF_ROOT=~/imgtec/mips-mti-elf/2015.06-05

Add to the path so the tools can be found e.g.
export PATH=$(MIPS_ELF_ROOT)/bin:$PATH

Then build the demo

cd freertos/FreeRTOS/Demo/lwIP_MIPS_GCC/mipsFPGA
make clean

make ARCH=m14kc 
