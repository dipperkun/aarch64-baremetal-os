ARCH ?= aarch64
TARGET = $(ARCH)-unknown-none-softfloat
CROSS_COMPILE ?= aarch64-linux-gnu-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -mcpu=cortex-a53 -march=armv8-a \
         -nostdlib -fno-builtin -fno-common \
         -Wall -Werror -O2 -g -I./include

ASFLAGS = $(CFLAGS)

LDFLAGS = -nostdlib -T linker.ld

C_SOURCES := $(shell find src -name "*.c")
ASM_SOURCES := $(shell find src -name "*.S")

C_OBJECTS := $(C_SOURCES:.c=.o)
ASM_OBJECTS := $(ASM_SOURCES:.S=.o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

IMAGE = os.elf
BIN = os.bin

all: $(BIN)

$(BIN): $(IMAGE)
	$(OBJCOPY) -O binary $< $@
	@echo "Built $@"

$(IMAGE): $(OBJECTS) linker.ld
	@echo "Linking $(IMAGE)..."
	$(LD) $(LDFLAGS) -o $@ $(filter-out linker.ld, $^)
	$(OBJDUMP) -d $(IMAGE) > os.dis
	@echo "Built $@"

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "Assembling $<..."
	$(CC) $(ASFLAGS) -c $< -o $@

run: $(BIN)
	qemu-system-aarch64 -machine virt -cpu cortex-a53 \
		-smp 4 -m 512M \
		-kernel $(IMAGE) \
		-nographic -serial mon:stdio

debug: $(IMAGE)
	qemu-system-aarch64 -machine virt -cpu cortex-a53 \
		-smp 4 -m 512M \
		-kernel $(IMAGE) \
		-nographic -serial mon:stdio -S -gdb tcp::1234

clean:
	rm -f $(OBJECTS) $(IMAGE) $(BIN) os.dis
	find . -name "*.o" -delete

distclean: clean
	rm -f os.dis

.PHONY: all run debug clean distclean
