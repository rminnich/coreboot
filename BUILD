#!/bin/bash
make
./util/cbfstool/cbfstool build/coreboot.rom add-payload -f bzImage -n fallback/rampayload -C earlyprintk=ttyS0,115200,keep
qemu-system-x86_64 -bios build/coreboot.rom -serial stdio -s
