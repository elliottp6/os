# enablesparallel compilation based on the # of processors
# MAKEFLAGS+=j$(shell nproc)

# kernel files
KERNEL_INCLUDES = -I./kernel/
KERNEL_ASM = $(shell find kernel -name "*.asm")
KERNEL_C = $(shell find kernel -name "*.c")
KERNEL_OBJ = $(KERNEL_ASM:%.asm=obj/%.o) $(KERNEL_C:%.c=obj/%.o)
KERNEL_FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# print out object files
# $(info $$KERNEL_OBJ is [${KERNEL_OBJ}])

# build virtual disk: boot.bin + kernel.bin
disk: bin/boot.bin bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin

# assembler bootloader
bin/boot.bin: boot/boot.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin

# link kernel objects
bin/kernel.bin: $(KERNEL_OBJ)
	mkdir -p bin
	i686-elf-ld -g -relocatable $(KERNEL_OBJ) -o obj/kernel/kernel.o
	i686-elf-gcc $(KERNEL_FLAGS) -T kernel/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel/kernel.o

# assemble kernel ASM files
obj/kernel/_start.o: kernel/_start.asm
	mkdir -p obj/kernel
	nasm -f elf -g kernel/_start.asm -o obj/kernel/_start.o

# compile kernel C files
obj/kernel/main.o: kernel/main.c
	mkdir -p obj/kernel
	i686-elf-gcc $(KERNEL_INCLUDES) $(KERNEL_FLAGS) -std=gnu99 -c kernel/main.c -o obj/kernel/main.o
