# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble bootloader and run it in qemu as the disk
bootloder: boot/boot.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin
	qemu-system-i386 -hda bin/boot.bin
	

