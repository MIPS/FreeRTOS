from CSUtils import DA
from CSUtils import DAtiny
from CSUtils import da_extensions
import time
import os


filename = "RTOSDemo.elf"
paddr    = 0x00000000

def main() :
	target_info = DA.GetTargetInfo()
	DAtiny.UseTarget(target_info[0])

	# Set VPE to thread 0.
	# If there are more threads, set it to thread 1
	VPE = DA.GetFirstThread()
	thread_count = DAtiny.GetThreadCount()
	if (thread_count > 1):
		VPE = DA.GetNextThread(VPE)
		DAtiny.SetThread(1)
	else:
		DAtiny.SetThread(0)

	if __name__ == "__main__" : DA.EnableTargetUpdates(VPE, 0)
	DA.HardReset()

	DA.SelectTarget(VPE)

	# Allow the bootloader to set things up for us
	print "Running bootloader...",
	DA.RunAllThreads(False)
	for i in range(5):
		time.sleep(float(1000) / 1000)
		print "%d " % (i+1),
	print ""
	DA.StopAllThreads(False)

	# Clear cause and status register to make sure no interrupts are pending
	# and exception bits are cleared
	DAtiny.WriteRegister("status", 0)
	DAtiny.WriteRegister("cause", 0)

	# ensure kseg0 is in cached mode
	config = DAtiny.ReadRegister("config")
	if ((config & 0x2 == 0x2) or (config & 0x7 == 0x7)):
		print "kseg0 is uncached, setting it to cacheable, noncoherent, write-back, write allocate"
#		config = (config & ~0x7) | 0x3
		config = (config & ~0x7) | 0x3
		DAtiny.WriteRegister("config", config)

	# Enable multi threading
	mt_supported = DAtiny.ReadRegister("config3") & 0x4
	if (mt_supported):
		DAtiny.WriteRegister("tchalt", 1);
		tmp = DAtiny.ReadRegister("mvpcontrol")
		tmp &= ~0x1	# EVP = 0
		tmp |= 0x2	# VPC = 1
		DAtiny.WriteRegister("mvpcontrol", tmp)
		tmp = DAtiny.ReadRegister("tcbind")
		tmp |= 1 << 21	# CurTC = 1
		tmp |= 0x1	# CurVPE = 1
		DAtiny.WriteRegister("tcbind", tmp)
		tmp = DAtiny.ReadRegister("vpeconf0")
		tmp |= 0x2	# MVP = 1
		tmp |= 0x1	# VPA = 1
		DAtiny.WriteRegister("vpeconf0", tmp)
		tmp = DAtiny.ReadRegister("tcstatus")
		tmp |= 1 << 13	# A = 1
		tmp &= ~(1 << 10) # IXMT = 0
		DAtiny.WriteRegister("tcstatus", tmp)
		tmp = DAtiny.ReadRegister("vpecontrol")
		tmp |= 1 << 15	# TE = 1
		DAtiny.WriteRegister("vpecontrol", tmp)
		tmp = DAtiny.ReadRegister("mvpcontrol")
		tmp |= 0x1	# EVP = 1
		tmp &= ~0x2	# VPC = 0
		DAtiny.WriteRegister("mvpcontrol", tmp)
		DAtiny.WriteRegister("tchalt", 0);

	print "Loading Symbols from " + filename
	da_extensions.LoadProgramFileEx(filename, False)

	if __name__ == "__main__" : DA.EnableTargetUpdates(VPE, 1)

if __name__ == "__main__" :
	main()
