# enablesparallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# kernel files
KERNEL_INCLUDES = -I./kernel/
KERNEL_ASM = $(shell find kernel -name "*.asm")
KERNEL_ASM_OBJ = $(KERNEL_ASM:%.asm=obj/%.o)
KERNEL_C = $(shell find kernel -name "*.c")
KERNEL_C_OBJ = $(KERNEL_C:%.c=obj/%.o)
KERNEL_FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# build virtual disk: boot.bin + kernel.bin
disk: bin/boot.bin bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin

# assembler bootloader, passing in the size of the kernel so it knows how many sectors to load
bin/boot.bin: boot/boot.asm bin/kernel.bin
	mkdir -p bin
	echo "kernel size equ $(wc -c < bin/kernel.bin)" > bin/kernel_size.inc
	nasm -f bin boot/boot.asm -o bin/boot.bin

# link kernel objects
bin/kernel.bin: $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ)
	mkdir -p bin
	x86_64-elf-ld -g -relocatable $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) -o obj/kernel.o
	x86_64-elf-gcc $(KERNEL_FLAGS) -T kernel/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel.o

# assemble kernel ASM files
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	mkdir -p $(dir $@)
	nasm -f elf64 -g $< -o $@

# compile kernel C files
$(KERNEL_C_OBJ): $(KERNEL_C)
	mkdir -p $(dir $@)
	x86_64-elf-gcc $(KERNEL_INCLUDES) $(KERNEL_FLAGS) -std=gnu99 -c $< -o $@

# clean up all the files/folders
clean:
	rm -rf bin
	rm -rf obj
