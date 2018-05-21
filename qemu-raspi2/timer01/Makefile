CROSS = arm-none-eabi
CC = ${CROSS}-gcc
AS = ${CROSS}-as
OBJDUMP = ${CROSS}-objdump
CFLAGS =  -mcpu=cortex-a7 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra
ASM_FLAGS = -mcpu=cortex-a7 
OBJ = boot.o kernel.o

kernel.elf: ${OBJ}
	${CC} -T linker.ld -o $@ -ffreestanding -O2 -nostdlib ${OBJ}
	${OBJDUMP} -D kernel.elf > kernel.list


boot.o: boot.S
	${AS} ${ASM_FLAGS} -c $< -o $@

%.o : %.c Makefile
	$(CC) ${CFLAGS} -c -o $*.o $*.c

run :
	$(MAKE) kernel.elf
	qemu-system-arm -M raspi2 -m 128 -serial mon:stdio -nographic -kernel kernel.elf

runasm :
	$(MAKE) kernel.elf
	qemu-system-arm -M raspi2 -m 128 -serial mon:stdio -nographic -kernel kernel.elf -d in_asm

clean:
	rm -f *.o *.elf *.list

.PHONY: clean
