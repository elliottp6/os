# this parameter enables parallel compilation based on the # of processors
MAKEFLAGS+=j$(shell nproc)

# assemble bootloader sector and run it in qemu as the disk
bootloader: boot/boot.asm
	mkdir -p bin
	nasm -f bin boot/boot.asm -o bin/boot.bin
	qemu-system-x86_64 -hda bin/boot.bin
	

