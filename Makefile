ARCH ?= aarch64
TARGET = $(ARCH)-unknown-none-softfloat
CROSS_COMPILE ?= aarch64-linux-gnu-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -mcpu=cortex-a53 -march=armv8-a \
         -nostdlib -fno-builtin -fno-common \
         -Wall -Werror -O2 -g

LDFLAGS = -nostdlib -T linker.ld

SOURCES = src/boot.S \
          src/main.c \
          src/uart.c \
          src/mmu.c \
          src/gicv3.c \
          src/scheduler.c \
          src/psci.c \
          src/task.c \
          src/irq_handler.c \
          src/timer.c

OBJECTS = $(SOURCES:.c=.o) $(SOURCES:.S=.o)

IMAGE = os.elf
BIN = os.bin

all: $(BIN)

$(BIN): $(IMAGE)
	$(OBJCOPY) -O binary $< $@
	@echo "Built $@"

$(IMAGE): $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(filter-out linker.ld, $^)
	$(OBJDUMP) -d $(IMAGE) > os.dis
	@echo "Built $@"

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/%.o: src/%.S
	$(CC) $(CFLAGS) -c $< -o $@

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

.PHONY: all run debug clean
