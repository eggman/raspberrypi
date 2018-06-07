CROSS = aarch64-elf
CC = ${CROSS}-gcc
AS = ${CROSS}-as
OBJDUMP = ${CROSS}-objdump
CFLAGS =  -mcpu=cortex-a53 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra
ASM_FLAGS = -mcpu=cortex-a53 
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
	qemu-system-aarch64 -M raspi3 -m 128 -serial null -serial mon:stdio -nographic -kernel kernel.elf -sd sd.img

runasm :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 128 -serial null -serial mon:stdio -nographic -kernel kernel.elf -d in_asm -sd sd.img

trun :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 128 -serial null -serial mon:stdio -nographic -kernel kernel.elf -sd sd.img -trace events=events

r
clean:
	rm -f *.o *.elf *.list

.PHONY: clean
