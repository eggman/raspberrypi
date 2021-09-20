
This simple example set generic timer to 1 sec on QEMU raspi3 model. 
This program only supports QEMU raspi3 model, and does not work on real hardware.


```
$ make run
qemu-system-aarch64 -M raspi3 -m 1024 -serial mon:stdio -nographic -kernel kernel.elf
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
timer01
CNTFRQ  : 0x3B9ACA0
CNTV_TVAL: 0x3B97F82
handler CNTV_TVAL: 0xFFFDFFCB
handler CNTVCT   : 0x3C18444
handler CNTV_TVAL: 0xFFFE14A5
handler CNTVCT   : 0x77D3EEA
```
