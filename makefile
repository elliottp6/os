# enablesparallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# kernel files
KERNEL_INCLUDES = -I./kernel/
KERNEL_SOURCES = $(shell find kernel -name "*.c" ! -wholename "*/trash/*")
KERNEL_OBJECTS = obj/start.o obj/main.o # $(C_FILES:./%.c=./obj/%.o)
KERNEL_FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# build virtual disk: boot.bin + kernel.bin
disk: bin/boot.bin bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin

# assembler bootloader
bin/boot.bin: boot/boot.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin

# link kernel objects
bin/kernel.bin: $(KERNEL_OBJECTS)
	mkdir -p bin
	i686-elf-ld -g -relocatable $(KERNEL_OBJECTS) -o obj/kernel.o
	i686-elf-gcc $(FLAGS) -T kernel/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel.o

# assemble kernel ASM files
obj/start.o: kernel/start.asm
	mkdir -p obj
	nasm -f elf -g kernel/start.asm -o obj/start.o

# compile kernel C files
obj/main.o: kernel/main.c
	mkdir -p obj
	i686-elf-gcc $(KERNEL_INCLUDES) $(FLAGS) -std=gnu99 -c kernel/main.c -o obj/main.o
