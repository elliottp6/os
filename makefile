# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# constants
INCLUDES = -I./
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# files (OBJECTS = $(SOURCES:%.cc=%.o))
C_FILES = $(shell find . -name "*.c" ! -wholename "*/trash/*")
OBJ_FILES = obj/start.o obj/main.o

# build OS disk by combining boot.bin with kernel.bin
os: bin/boot.bin bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin

# assembler bootloader
bin/boot.bin: boot/boot.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin

# link kernel
bin/kernel.bin: $(OBJ_FILES)
	mkdir -p bin
	i686-elf-ld -g -relocatable $(OBJ_FILES) -o obj/kernel.o
	i686-elf-gcc $(FLAGS) -T kernel/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel.o

# assemble kernel start
obj/start.o: kernel/start.asm
	mkdir -p obj
	nasm -f elf -g kernel/start.asm -o obj/start.o

# compile C files
obj/main.o: kernel/main.c
	mkdir -p obj
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c kernel/main.c -o obj/main.o
