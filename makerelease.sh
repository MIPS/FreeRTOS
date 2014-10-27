# Simply tar our port and demo from the head of mips32 branch
set -eu

TAG=`git describe --tags $(git rev-list --tags --max-count=1)`
DATE=`date +%d%b%Y`
RELEASE_ZIP=$TAG-mips32-$DATE.zip
RELEASE_NOTES=ReleaseNotes.txt

CS_SDK_LINK=http://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/download-codescape-mips-sdk-essentials/

# zip the release
git archive --format=zip mips32			\
	FreeRTOS/Source/portable/GCC/MIPS32/	\
	FreeRTOS/Demo/MIPS32_GCC/		\
	> $RELEASE_ZIP

# Create ReleaseNotes file
cat << EOF > $RELEASE_NOTES
-------------------------------------
$RELEASE_ZIP
-------------------------------------

This port provides support for MIPS32 cores.


Requirements:

This port is dependent on features provided by Codescape SDK.
To get the latest version visit the following website:

   $CS_SDK_LINK

You must use SDK v1.3 or higher.

Or if you're just installing the tools on their own, it must be v2015.06-xx
or higher.

You will also need $TAG official release which this release must be extracted on top of.


Features:
   * Vectored interrupt, GIC and EIC support.
   * Count/Compare register based internal timer support.
   * DSP and FPU context switching.
   * microMips support.
   * Cold boot support


New Changes:
   * The new SDK has a Hardware Abstraction Layer (HAL) that implements the
     Unified Hosting Interface (UHI) and compiler support for creating
     interrupt handlers.

   * Global Interrupt Controller (GIC) support.
     GIC allows more interrupts to be connected to the core.

   * Experimental External Interrupt Controller (EIC) support.
     EIC mode transforms the 6 core interrupt lines into 64 priority
     encoded ones.
     This could be provided by GIC or some other external interrupt
     controller.
     This received very little testing, so use with caution.

   * Baseline interrupt support was removed since the toolchain
     doesn't have instrinsic support for it.
     Vectored interrupt is the norm on all new cores.

   * DSP and FPU context switching support.

   * Can't mix MIPS32 and microMips exception handling anymore.

   * Now you can make FreeRTOS romable by including boot code in the binary.
     The boot code can either be combined in the same binary as FreeRTOS
     or built separately as a first stage bootloader.

     The support for this is dependent on the SDK which provides generic support for
     cold boot. If you're interested in this feature you might want to consult the
     documentation installed with the SDK.

     Note that the boot code in the SDK has a different licence to the FreeRTOS.
     Consult the SDK documentation for more info.

   * New config based build system.
     You can rebuild the system for various configurations by applying
     different config files instead of passing command line arguments.
     To apply a config file

       make -C FreeRTOS/Demo/MIPS32_GCC/ myconfigfile

    config files must be in FreeRTOS/Demo/MIPS32_GCC/configs


Building the Demo:
   * make -C FreeRTOS/Demo/MIPS32_GCC/ defaultconfig
   * make -C FreeRTOS/Demo/MIPS32_GCC/

NOTE: The compiler must be in your path. You'll need mips-mti-elf- toolchain
for mips32r2-r5 targets and mips-img-elf- for mips32r6 targets.
EOF

# Add ReleasetNotes.txt to the zip file
zip $RELEASE_ZIP $RELEASE_NOTES 1&>/dev/null
