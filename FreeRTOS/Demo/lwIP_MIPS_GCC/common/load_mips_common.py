from imgtec import codescape
import time
import os, sys

def setup_and_load(filename):
	if codescape.environment == 'standalone':
	    # Connect to the probe passed on the command line
	    probe = codescape.ConnectProbe(sys.argv[1])
	    core = probe.cores[0]
	    thread = probe.hwthreads[0]
	else:
	    thread = codescape.GetSelectedThread()
	    probe = thread.probe
	    core = probe.cores[0]

	core.HardReset()

	# Allow the bootloader to set things up for us
	print "Running bootloader...",
	core.RunAll(False)
	for i in range(8):
		time.sleep(float(1000) / 1000)
		print "%d " % (i+1),
	print ""
	core.StopAll(False)

	# Clear cause and status register to make sure no interrupts are pending
	# and exception bits are cleared
	thread.WriteRegister("status", 0)
	thread.WriteRegister("cause", 0)

	# ensure kseg0 is in cached mode
	config = thread.ReadRegister("config")
	print hex(config)
	if (((config & 0x7) == 0x2) or ((config & 0x7) == 0x7)):
		print "kseg0 is uncached, setting it to cacheable, noncoherent, write-back, write allocate"
		config = (config & ~0x7) | 0x3
		thread.WriteRegister("config", config)

	print "Loading Program + Symbols from " + filename
	probe.EnableAllMemory()
	thread.LoadProgramFile(filename, False, 0x403) #flags = 0x403 == load all + extend HSP with elf sections
