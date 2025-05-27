# Target and build settings
TARGET     := kernel.elf
ISO        := os.iso
BUILD_DIR  := build
PROGRAM    := iso/modules/program

# Compiler and linker settings
CC         := gcc
LD         := ld
AS         := nasm

# Compiler and linker flags
CFLAGS     := -g -m32 -nostdlib -fno-builtin -fno-stack-protector \
              -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I. -I./include
ASFLAGS    := -f elf32
LDFLAGS    := -T link.ld -melf_i386 -O2 -nostdlib

# User program flags
USER_CFLAGS := -m32 -fno-builtin -fno-stack-protector -fno-pic -c -I./include
USER_LDFLAGS := -T app/link.ld -melf_i386

# Directories
ISO_DIR    := iso
BOOT_DIR   := $(ISO_DIR)/boot
GRUB_DIR   := $(BOOT_DIR)/grub
MODULES_DIR := $(ISO_DIR)/modules

# Source files - exclude the tools directory
ASM_SOURCES := $(shell find . -name '*.s' -not -path "./build/*" -not -path "./include/*" -not -path "./iso/modules/*" -not -path "./app/*" -not -path "./tools/*")
C_SOURCES   := $(shell find . -name '*.c' -not -path "./build/*" -not -path "./include/*" -not -path "./app/*" -not -path "./tools/*")

# Object files
ASM_OBJECTS := $(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
C_OBJECTS   := $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
OBJECTS     := $(ASM_OBJECTS) $(C_OBJECTS)

# Dependency files
DEPS        := $(C_SOURCES:%.c=$(BUILD_DIR)/%.d)

# QEMU settings
QEMU        := qemu-system-i386
QEMU_FLAGS  := -boot d -no-reboot -no-shutdown -d int -D qemu.log

# User program sources
USER_C_SOURCES := app/main.c app/syscall.c
USER_ASM_SOURCES := app/start.s
USER_OBJECTS := $(USER_C_SOURCES:%.c=$(BUILD_DIR)/%.o) $(USER_ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)

# Tool for creating initrd
TOOLS_DIR := tools
INITRD_TOOL := $(TOOLS_DIR)/create_initrd
INITRD_IMAGE := initrd.img

.PHONY: all clean run gui nographic gdb initrd

all: $(ISO) initrd

# Build the initrd tool using host compiler with standard libraries
$(INITRD_TOOL): $(TOOLS_DIR)/create_initrd.c
	@echo "Building initrd tool..."
	$(CC) -o $@ $<

# Create the initrd image
initrd: $(INITRD_TOOL)
	@echo "Creating initrd image..."
	./create_test_initrd.sh

$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(TARGET) $(PROGRAM) initrd
	@echo "Creating ISO image..."
	@mkdir -p $(GRUB_DIR)
	cp stage2_eltorito $(GRUB_DIR)/
	cp $(TARGET) $(BOOT_DIR)/kernel.elf
	cp menu.lst $(GRUB_DIR)/
	@echo "Copying initrd.img to boot directory..."
	@cp -v $(INITRD_IMAGE) $(BOOT_DIR)/
	genisoimage -R -b boot/grub/stage2_eltorito \
	            -no-emul-boot -boot-load-size 4 \
	            -A os -input-charset utf8 -quiet \
	            -boot-info-table -o $@ $(ISO_DIR)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -MMD -MP -MF $(BUILD_DIR)/$*.d -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	$(AS) $(ASFLAGS) $< -o $@

# User program build rules
$(PROGRAM): $(USER_OBJECTS)
	@echo "Linking user program..."
	@mkdir -p $(MODULES_DIR)
	$(LD) $(USER_LDFLAGS) -o $@ $^

$(BUILD_DIR)/app/%.o: app/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling user program $<..."
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/app/%.o: app/%.s
	@mkdir -p $(dir $@)
	@echo "Assembling user program $<..."
	$(AS) $(ASFLAGS) $< -o $@

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(TARGET) $(ISO) $(INITRD_TOOL) $(INITRD_IMAGE) *.d *.log
	@echo "Done!"

run: nographic

gui: $(ISO)
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO)

nographic: $(ISO)
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO) -nographic

gdb: $(ISO)
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO) -s -S -nographic

# Include dependency files
-include $(DEPS)
