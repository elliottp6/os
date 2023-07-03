# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble OS by combining boot.bin with kernel.bin
os: boot/boot.asm kernel/kernel.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin
	nasm -f bin kernel/kernel.asm -o bin/kernel.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin
