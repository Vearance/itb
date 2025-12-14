# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = HuTaOS
DISK_NAME     = storage
BG_DIR        = media/bg
BG_IMAGES     = $(wildcard $(BG_DIR)/*.png) $(wildcard $(BG_DIR)/*.jpg) $(wildcard $(BG_DIR)/*.jpeg) $(wildcard $(BG_DIR)/*.webp)
BG_NAMES      = $(notdir $(basename $(BG_IMAGES)))
BG_BINS       = $(addprefix $(OUTPUT_FOLDER)/bg/,$(addsuffix .bin,$(BG_NAMES)))
BG_BINS_REL   = $(patsubst $(OUTPUT_FOLDER)/%,%,$(BG_BINS))
HUTAO_SOURCE  = media/hutao
HUTAO_DIR     = hutao/hutao/hutao/hutao/hutao/hutao/hutao/hutao/hutao/hutao
HUTAO_TARGET  = $(HUTAO_DIR)/hutao
HUTAO_STAGE   = $(OUTPUT_FOLDER)/insert-assets

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

C_SOURCES := $(shell find $(SOURCE_FOLDER) -name "*.c" -not -path "$(SOURCE_FOLDER)/c/inserter/*" -not -path "$(SOURCE_FOLDER)/c/usermode/*")
C_OBJECTS := $(patsubst $(SOURCE_FOLDER)/%.c, $(OUTPUT_FOLDER)/src/%.o, $(C_SOURCES))

run: all
	@qemu-system-i386 -s -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0 -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
run-no-audio: all
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: build disk insert-shell insert-clock insert-hello insert-badapple insert-bg insert-hutao
build: iso inserter 
clean:
	rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel

disk:
	@if [ ! -f $(OUTPUT_FOLDER)/$(DISK_NAME).bin ]; then \
		qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 512M; \
		echo "Created new disk image."; \
	else \
		echo "Using existing disk image (preserving saved data)."; \
	fi

disk-reset:
	@rm -f $(OUTPUT_FOLDER)/$(DISK_NAME).bin
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 512M
	@echo "Disk image reset to empty."

kernel: $(C_OBJECTS) $(OUTPUT_FOLDER)/kernel-entrypoint.o $(OUTPUT_FOLDER)/intsetup.o $(OUTPUT_FOLDER)/context-switch.o
	@$(LIN) $(LFLAGS) $(C_OBJECTS) $(OUTPUT_FOLDER)/kernel-entrypoint.o $(OUTPUT_FOLDER)/intsetup.o $(OUTPUT_FOLDER)/context-switch.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...

$(OUTPUT_FOLDER)/kernel-entrypoint.o: $(SOURCE_FOLDER)/kernel-entrypoint.s
	@$(ASM) $(AFLAGS) $< -o $@

$(OUTPUT_FOLDER)/intsetup.o: $(SOURCE_FOLDER)/c/interrupt/intsetup.s
	@$(ASM) $(AFLAGS) $< -o $@

$(OUTPUT_FOLDER)/context-switch.o: $(SOURCE_FOLDER)/context-switch.s
	@$(ASM) $(AFLAGS) $< -o $@

$(C_OBJECTS): $(OUTPUT_FOLDER)/src/%.o: $(SOURCE_FOLDER)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -o $@

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@cd $(OUTPUT_FOLDER) && genisoimage -R \
		-b boot/grub/grub1 \
		-no-emul-boot \
		-boot-load-size 4 \
		-A "$(ISO_NAME)" \
		-input-charset utf-8 \
		-quiet \
		-boot-info-table \
		-o $(ISO_NAME).iso iso
	@rm -r $(OUTPUT_FOLDER)/iso/

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
	$(SOURCE_FOLDER)/c/stdlib/string.c \
	$(SOURCE_FOLDER)/c/driver/ext2.c \
	$(SOURCE_FOLDER)/c/inserter/external-inserter.c \
	-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@mkdir -p $(OUTPUT_FOLDER)/user-shell
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/c/usermode/crt0.s -o $(OUTPUT_FOLDER)/user-shell/crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/stdlib/string.c -o $(OUTPUT_FOLDER)/user-shell/string.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/cpu/portio.c -o $(OUTPUT_FOLDER)/user-shell/portio.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/driver/disk.c -o $(OUTPUT_FOLDER)/user-shell/disk.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/driver/ext2.c -o $(OUTPUT_FOLDER)/user-shell/ext2.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/graphics/graphics.c -o $(OUTPUT_FOLDER)/user-shell/graphics.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/user-shell.c -o $(OUTPUT_FOLDER)/user-shell/user-shell.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/user-heap.c -o $(OUTPUT_FOLDER)/user-shell/user-heap.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/cd.c -o $(OUTPUT_FOLDER)/user-shell/cd.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/ls.c -o $(OUTPUT_FOLDER)/user-shell/ls.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/mkdir.c -o $(OUTPUT_FOLDER)/user-shell/mkdir.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/cat.c -o $(OUTPUT_FOLDER)/user-shell/cat.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/rm.c -o $(OUTPUT_FOLDER)/user-shell/rm.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/cp.c -o $(OUTPUT_FOLDER)/user-shell/cp.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/mv.c -o $(OUTPUT_FOLDER)/user-shell/mv.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/find.c -o $(OUTPUT_FOLDER)/user-shell/find.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/grep.c -o $(OUTPUT_FOLDER)/user-shell/grep.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/touch.c -o $(OUTPUT_FOLDER)/user-shell/touch.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/echo.c -o $(OUTPUT_FOLDER)/user-shell/echo.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/exec.c -o $(OUTPUT_FOLDER)/user-shell/exec.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/ps.c -o $(OUTPUT_FOLDER)/user-shell/ps.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/kill.c -o $(OUTPUT_FOLDER)/user-shell/kill.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/commands/bg.c -o $(OUTPUT_FOLDER)/user-shell/bg.o
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/user-linker.ld -melf_i386 --oformat=binary \
		$(OUTPUT_FOLDER)/user-shell/crt0.o $(OUTPUT_FOLDER)/user-shell/string.o $(OUTPUT_FOLDER)/user-shell/portio.o $(OUTPUT_FOLDER)/user-shell/disk.o $(OUTPUT_FOLDER)/user-shell/ext2.o $(OUTPUT_FOLDER)/user-shell/graphics.o $(OUTPUT_FOLDER)/user-shell/user-shell.o $(OUTPUT_FOLDER)/user-shell/user-heap.o $(OUTPUT_FOLDER)/user-shell/cd.o $(OUTPUT_FOLDER)/user-shell/ls.o $(OUTPUT_FOLDER)/user-shell/mkdir.o $(OUTPUT_FOLDER)/user-shell/cat.o $(OUTPUT_FOLDER)/user-shell/rm.o $(OUTPUT_FOLDER)/user-shell/cp.o $(OUTPUT_FOLDER)/user-shell/mv.o $(OUTPUT_FOLDER)/user-shell/find.o $(OUTPUT_FOLDER)/user-shell/grep.o $(OUTPUT_FOLDER)/user-shell/touch.o $(OUTPUT_FOLDER)/user-shell/echo.o $(OUTPUT_FOLDER)/user-shell/exec.o $(OUTPUT_FOLDER)/user-shell/ps.o $(OUTPUT_FOLDER)/user-shell/kill.o $(OUTPUT_FOLDER)/user-shell/bg.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		$(OUTPUT_FOLDER)/user-shell/crt0.o $(OUTPUT_FOLDER)/user-shell/string.o $(OUTPUT_FOLDER)/user-shell/portio.o $(OUTPUT_FOLDER)/user-shell/disk.o $(OUTPUT_FOLDER)/user-shell/ext2.o $(OUTPUT_FOLDER)/user-shell/graphics.o $(OUTPUT_FOLDER)/user-shell/user-shell.o $(OUTPUT_FOLDER)/user-shell/user-heap.o $(OUTPUT_FOLDER)/user-shell/cd.o $(OUTPUT_FOLDER)/user-shell/ls.o $(OUTPUT_FOLDER)/user-shell/mkdir.o $(OUTPUT_FOLDER)/user-shell/cat.o $(OUTPUT_FOLDER)/user-shell/rm.o $(OUTPUT_FOLDER)/user-shell/cp.o $(OUTPUT_FOLDER)/user-shell/mv.o $(OUTPUT_FOLDER)/user-shell/find.o $(OUTPUT_FOLDER)/user-shell/grep.o $(OUTPUT_FOLDER)/user-shell/touch.o $(OUTPUT_FOLDER)/user-shell/echo.o $(OUTPUT_FOLDER)/user-shell/exec.o $(OUTPUT_FOLDER)/user-shell/ps.o $(OUTPUT_FOLDER)/user-shell/kill.o $(OUTPUT_FOLDER)/user-shell/bg.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...	
	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -rf $(OUTPUT_FOLDER)/user-shell/*.o

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 1 $(DISK_NAME).bin

user-clock:
	@mkdir -p $(OUTPUT_FOLDER)/user-clock
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/c/usermode/programs/clock/crt0-clock.s -o $(OUTPUT_FOLDER)/user-clock/crt0-clock.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/stdlib/string.c -o $(OUTPUT_FOLDER)/user-clock/string.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/cpu/portio.c -o $(OUTPUT_FOLDER)/user-clock/portio.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/programs/clock/clock.c -o $(OUTPUT_FOLDER)/user-clock/clock.o
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/programs/clock/clock-linker.ld -melf_i386 --oformat=binary \
		$(OUTPUT_FOLDER)/user-clock/crt0-clock.o $(OUTPUT_FOLDER)/user-clock/string.o $(OUTPUT_FOLDER)/user-clock/portio.o $(OUTPUT_FOLDER)/user-clock/clock.o -o $(OUTPUT_FOLDER)/clock
	@echo Linking object clock object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/programs/clock/clock-linker.ld -melf_i386 --oformat=elf32-i386 \
		$(OUTPUT_FOLDER)/user-clock/crt0-clock.o $(OUTPUT_FOLDER)/user-clock/string.o $(OUTPUT_FOLDER)/user-clock/portio.o $(OUTPUT_FOLDER)/user-clock/clock.o -o $(OUTPUT_FOLDER)/clock_elf
	@echo Linking object clock object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/clock
	@rm -rf $(OUTPUT_FOLDER)/user-clock/*.o

insert-clock: inserter user-clock
	@echo Inserting clock into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter clock 1 $(DISK_NAME).bin

user-hello:
	@mkdir -p $(OUTPUT_FOLDER)/user-hello
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/c/usermode/programs/hello/crt0-hello.s -o $(OUTPUT_FOLDER)/user-hello/crt0-hello.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/c/usermode/programs/hello/hello.c -o $(OUTPUT_FOLDER)/user-hello/hello.o
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/programs/hello/hello-linker.ld -melf_i386 --oformat=binary \
		$(OUTPUT_FOLDER)/user-hello/crt0-hello.o $(OUTPUT_FOLDER)/user-hello/hello.o -o $(OUTPUT_FOLDER)/hello
	@echo Linking object hello object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/c/usermode/programs/hello/hello-linker.ld -melf_i386 --oformat=elf32-i386 \
		$(OUTPUT_FOLDER)/user-hello/crt0-hello.o $(OUTPUT_FOLDER)/user-hello/hello.o -o $(OUTPUT_FOLDER)/hello_elf
	@echo Linking object hello object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/hello
	@rm -rf $(OUTPUT_FOLDER)/user-hello/*.o

insert-hello: inserter user-hello
	@echo Inserting hello program into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter hello 1 $(DISK_NAME).bin

insert-badapple: inserter
	@echo Inserting Bad Apple frames into root directory...
	@mkdir -p $(OUTPUT_FOLDER)/badapple
	@cp media/badapple/badapple.bin $(OUTPUT_FOLDER)/badapple/badapple.bin; cd $(OUTPUT_FOLDER); ./inserter badapple/badapple.bin 1 $(DISK_NAME).bin
	@cp media/badapple/badapple.pcm $(OUTPUT_FOLDER)/badapple/badapple.pcm; cd $(OUTPUT_FOLDER); ./inserter badapple/badapple.pcm 1 $(DISK_NAME).bin
	@rm $(OUTPUT_FOLDER)/badapple/badapple.bin $(OUTPUT_FOLDER)/badapple/badapple.pcm

insert-hutao: inserter disk
	@echo Inserting hutao file into nested directory...
	@mkdir -p $(HUTAO_STAGE)/$(HUTAO_DIR)
	@cp $(HUTAO_SOURCE) $(HUTAO_STAGE)/$(HUTAO_TARGET)
	@cd $(HUTAO_STAGE); ../inserter $(HUTAO_TARGET) 1 ../$(DISK_NAME).bin
	@rm -rf $(HUTAO_STAGE)

# Convert all images in media/bg into raw 320x200x8bpp binaries
bg-bins: $(BG_BINS)

$(OUTPUT_FOLDER)/bg/%.bin: $(BG_DIR)/%.png
	@mkdir -p $(dir $@)
	@python3 scripts/wallpaper-to-bin.py $< $@

$(OUTPUT_FOLDER)/bg/%.bin: $(BG_DIR)/%.jpg
	@mkdir -p $(dir $@)
	@python3 scripts/wallpaper-to-bin.py $< $@

$(OUTPUT_FOLDER)/bg/%.bin: $(BG_DIR)/%.jpeg
	@mkdir -p $(dir $@)
	@python3 scripts/wallpaper-to-bin.py $< $@

$(OUTPUT_FOLDER)/bg/%.bin: $(BG_DIR)/%.webp
	@mkdir -p $(dir $@)
	@python3 scripts/wallpaper-to-bin.py $< $@

# Insert converted wallpapers into root of the storage image (inode 1)
insert-bg: inserter bg-bins disk
	@echo Inserting wallpapers from $(BG_DIR) into storage...
	@cd $(OUTPUT_FOLDER); for f in $(BG_BINS_REL); do \
		echo "  -> $$f"; ./inserter $$f 1 $(DISK_NAME).bin; \
	done