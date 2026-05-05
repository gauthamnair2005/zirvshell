CC      := x86_64-elf-gcc
LD      := x86_64-elf-ld
OBJCOPY := x86_64-elf-objcopy

# Fall back to system toolchain if cross-compiler is missing
ifeq (, $(shell which $(CC) 2>/dev/null))
  CC := gcc
  LD := ld
endif

CFLAGS := \
    -std=c11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-pic \
    -mno-red-zone \
    -Wall -Wextra -O2 \
    -Iinclude \
    -I../libs/zirvlibc/include

LDFLAGS := \
    -nostdlib \
    -static \
    -no-pie \
    -z max-page-size=0x1000

# MOSIX init must be statically linked against zirvlibc
LIBC_OBJS := \
    ../build/libs/zirvlibc/src/string.o \
    ../libs/zirvlibc/src/stdio_user.o \
    ../libs/zirvlibc/src/unistd_user.o \
    ../libs/zirvlibc/src/syscall_user.o \
    ../build/libs/zirvlibc/src/ctype.o

SRCS := src/main.c src/crt0.asm
OBJS := src/main.o src/crt0.o

TARGET := zirvinit.elf

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBC_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.asm
	nasm -f elf64 -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
