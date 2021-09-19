
This simple example uart rx interrupt on QEMU raspi3 model. 
This program only supports QEMU raspi3 model, and does not work on real hardware.

```
$ make run
qemu-system-aarch64 -M raspi3 -m 1024 -serial mon:stdio -nographic -kernel kernel.elf
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
int01
a c_irq_handler
b c_irq_handler
c c_irq_handler
```
