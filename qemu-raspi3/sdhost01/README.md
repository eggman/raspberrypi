
This simple example sdhost init sdcard on QEMU raspi3 model. 
This program only supports QEMU raspi3 model, and does not work on real hardware.

```
$ make run
qemu-system-aarch64 -M raspi3 -m 1024 -serial mon:stdio -nographic -kernel kernel.elf -drive if=sd,id=sd0,format=raw,file=sd.img
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
sdhost01
cmd  0 arg 0
resp 0 0 0 0
cmd  8 arg 1AA
resp 0 0 0 1AA
cmd  37 arg 0
resp 0 0 0 120
cmd  29 arg 51FF8000
resp 0 0 0 80FFFF00
cmd  2 arg 0
resp AA585951 454D5521 1DEADBE EF006219
```
