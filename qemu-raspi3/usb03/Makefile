CROSS_COMPILE = aarch64-linux-gnu-
CC = ${CROSS_COMPILE}gcc
AS = ${CROSS_COMPILE}as
OBJDUMP = ${CROSS_COMPILE}objdump
CFLAGS =  -mcpu=cortex-a53 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra
ASM_FLAGS = -mcpu=cortex-a53 
OBJ = boot.o kernel.o

kernel.elf: ${OBJ}
	${CC} -Wl,--build-id=none -T linker.ld -o $@ -ffreestanding -O2 -nostdlib ${OBJ}
	${OBJDUMP} -D kernel.elf > kernel.list


boot.o: boot.S
	${AS} ${ASM_FLAGS} -c $< -o $@

%.o : %.c Makefile
	$(CC) ${CFLAGS} -c -o $*.o $*.c

run :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf

runasm :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf -d in_asm

trun :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 1024 -serial null -serial mon:stdio -nographic -device usb-kbd -kernel kernel.elf -trace events=events

clean:
	rm -f *.o *.elf *.list

.PHONY: clean
