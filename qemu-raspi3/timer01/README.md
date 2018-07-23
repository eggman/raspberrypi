
This simple example set generic timer to 1 sec on QEMU raspi3 model. 
This program only supports QEMU raspi3 model, and does not work on real hardware.


```
$ make run
qemu-system-aarch64 -M raspi3 -m 128 -serial mon:stdio -nographic -kernel kernel.elf
CNTFRQ  : 0x3B9ACA0
CNTV_TVAL: 0x3B73210
handler CNTV_TVAL: 0xFFFB877F
handler CNTVCT   : 0x3CAB4AE
handler CNTV_TVAL: 0xFFFEAC90
handler CNTVCT   : 0x7856987
handler CNTV_TVAL: 0xFFFDC488
handler CNTVCT   : 0xB41BD99
```
