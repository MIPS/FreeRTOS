# MIPS FreeRTOS

This is a fork of FreeRTOS providing support for mips32 compliant cores from
mip32r2 to mips32r6 and includes support for the microMIPS ISA as well.

The freertos-trunk branch tracks the freertos svn repo. mips support is added on
the mips branch.

Requirements:

This port is dependent on features provided by Codescape SDK.
To get the latest version visit the following website:

http://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

You must use SDK v1.3 or higher.

Or if you're just installing the tools on their own, it must be v2015.06-xx
or higher.

Features:
* Uses FreeRTOS V9.0.0
* Demos for both CI40 and Xilinx NEXYS4DDR fpga board


NB. The Xilinx NEXYS4DDR fpga board must be programmed with a suitable bitfile
supporting a mips m14kc processor.

Building the Demo for the CI40 target:
```
cd FreeRTOS/Demo/lwIP_MIPS_GCC/CI40/
ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make
```

Building the Demo for the fpga target:
```
cd FreeRTOS/Demo/lwIP_MIPS_GCC/mipsFPGA/
ARCH=m14kc make
```

NOTE: The compiler must be in your path. You'll need mips-mti-elf- toolchain
for mips32r2-r5 targets and mips-img-elf- for mips32r6 targets.
