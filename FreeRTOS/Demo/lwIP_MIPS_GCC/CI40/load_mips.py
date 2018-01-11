import os,sys

if __name__ == "__main__":
    sys.path.insert(0, os.path.join(os.getcwd(),'..', 'common'))
    from load_mips_common import *

def main():
    setup_and_load("RTOSDemo.elf")

if __name__ == "__main__" :
    main()
