#!/bin/bash
set -e
(cd payloads/ram &&make)
make
echo if you comment the next line out then it does not work
#./util/cbfstool/cbfstool build/coreboot.rom add-payload -f payloads/ram/build/ram.elf -n fallback/rampayload -C earlyprintk=ttyS0,115200,keep
./util/cbfstool/cbfstool build/coreboot.rom add-payload -f bzImage -n fallback/linux -C earlyprintk=ttyS0,115200,keep
qemu-system-x86_64 -bios build/coreboot.rom -serial stdio -s
