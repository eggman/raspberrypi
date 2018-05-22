
Raspberry Pi bare metal code.

I use QEMU 2.12 raspi3 model and raspi2 model.

```
$ qemu-system-aarch64 -m 128 -M raspi3 -nographic -kernel kernel.elf
```

```
$ qemu-system-arm -m 128 -M raspi2 -nographic -kernel kernel.elf
```
