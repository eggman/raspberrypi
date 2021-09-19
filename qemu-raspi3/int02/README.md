
This simple example miniuart rx interrupt on QEMU raspi3 model. 
This program only supports QEMU raspi3 model, and does not work on real hardware.

```
$ make run
qemu-system-aarch64 -M raspi3 -m 128 -serial null -serial mon:stdio -nographic -kernel kernel.elf
qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c
int02
a c_irq_handler
b c_irq_handler
c c_irq_handler
```
