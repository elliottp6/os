# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble boot sectors and kernel, and them put it all into a single disk
# sector0 is boot1, sector1 is boot2, and then sector2+ is the OS kernel
bootloader: boot/boot1.asm boot/boot2.asm
	mkdir -p bin
	nasm -f bin boot/boot1.asm -o bin/boot1.bin
	nasm -f bin boot/boot2.asm -o bin/boot2.bin
	nasm -f bin kernel/kernel.asm -o bin/kernel.bin
	cat bin/boot1.bin bin/boot2.bin > bin/boot.bin
	cat bin/boot.bin bin/kernel.bin > bin/disk.bin
	qemu-system-x86_64 -hda bin/disk.bin
