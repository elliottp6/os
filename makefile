# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble OS by combining boot.bin with kernel.bin
os: boot/boot.asm kernel/start.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin
	nasm -f bin kernel/start.asm -o bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin

# assemble bootloader
#boot/boot.bin: 

# link kernel
#bin/kernel.bin: $(FILES)
#	i686-elf-ld -g -relocatable $(FILES) -o build/kernel.o
#	i686-elf-gcc $(FLAGS) -T src/linker.ld -o bin/kernel.bin -ffreestanding -O0 -nostdlib obj/kernel.o

# compile kernel main (and other C files)
#build/main.o: kernel/main.c
#	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c kernel/main.c -o obj/main.o
