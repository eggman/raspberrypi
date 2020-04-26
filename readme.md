
Raspberry Pi bare metal code.


# qemu-raspi2

| name     |               |
| -------- | ------------- |
| [framebuffer01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi2/framebuffer01)  | draw framebuffer  |
| [int01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi2/int01)    | uart rx interrupt handling |
| [timer01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi2/timer01)  | arm generic timer every 1 secnod  |

# qemu-raspi3

| name     |               |
| -------- | ------------- |
| [dma01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/dma01)       | dma  memory to meory |
| [framebuffer01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/framebuffer01)  | draw framebuffer  |
| [int01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/int01)       | uart rx interrupt handling |
| [int02](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/int02)       | mini uart rx interrupt handling |
| [sdhost01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/sdhost01) | sdhost initialize |
| [sdhost02](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/sdhost02) | sdhost readblock |
| [task01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/task01)     | task create  |
| [task02](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/task01)     | task switch  |
| [timer01](https://github.com/eggman/raspberrypi/tree/master/qemu-raspi3/timer01)   | arm generic timer every 1 secnod  |


# toolchain

```
$ sudo apt install gcc-arm-none-eabi
```

```
$ sudo apt install gcc-aarch64-linux-gnu
```

# qemu

I use QEMU 4.2 raspi3 model and raspi2 model.

```
$ qemu-system-aarch64 -m 128 -M raspi3 -nographic -kernel kernel.elf
```

```
$ qemu-system-arm -m 128 -M raspi2 -nographic -kernel kernel.elf
```
