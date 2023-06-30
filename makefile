# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble 1st & 2nd stage bootloaders, and write them into a single disk
bootloader: boot/boot1.asm boot/boot2.asm
	mkdir -p bin
	nasm -f bin boot/boot1.asm -o bin/boot1.bin
	nasm -f bin boot/boot2.asm -o bin/boot2.bin
	cat bin/boot1.bin bin/boot2.bin > boot.bin
	qemu-system-x86_64 -hda bin/boot.bin
