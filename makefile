# enablesparallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# kernel constants
KERNEL_ASM = $(shell find kernel -name "*.asm")
KERNEL_C = $(shell find kernel -name "*.c")
KERNEL_OBJ = $(KERNEL_ASM:%.asm=obj/%.o) $(KERNEL_C:%.c=obj/%.o)
KERNEL_INCLUDES = -I./kernel/
KERNEL_FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# build OS
os: bin/boot.bin bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin -display gtk,zoom-to-fit=on

# assembler bootloader
bin/boot.bin: boot/boot.asm bin/kernel.bin
	nasm -f bin boot/boot.asm -o bin/boot.bin

# link kernel objects, pad kernel to next 512 byte boundary, write the KERNEL_SECTORS to bin/kernel_sectors.inc for boot/boot.asm
bin/kernel.bin: $(KERNEL_OBJ)
	mkdir -p bin
	x86_64-elf-ld -g -relocatable $(KERNEL_OBJ) -o obj/kernel.o
	x86_64-elf-gcc $(KERNEL_FLAGS) -T kernel/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel.o
	kernel_size=$$(wc -c < bin/kernel.bin); \
	kernel_padding=$$(( (512 - ($$kernel_size % 512)) % 512 )); \
	kernel_sectors=$$((( $$kernel_size + $$kernel_padding ) / 512 )); \
	dd if=/dev/zero bs=1 count=$$kernel_padding >> bin/kernel.bin; \
	echo "KERNEL_SECTORS equ $$kernel_sectors" > bin/kernel_sectors.inc

# assemble kernel ASM files
obj/%.o: %.asm
	mkdir -p $(dir $@)
	nasm -f elf64 -g $< -o $@

# compile kernel C files
obj/%.o: %.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc $(KERNEL_INCLUDES) $(KERNEL_FLAGS) -std=gnu99 -c $< -o $@

# clean up all the files/folders
clean:
	rm -rf bin
	rm -rf obj
