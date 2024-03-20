.PHONY: koalaos

.ONESHELL:

CROSS_PREFIX := mipsel-linux-gnu

CC := gcc
CFLAGS := -g -DLOG_USE_COLOR -Ofast -lSDL2 -lSDL2main

FW_SOURCES := boot/reset.s
FW_SOURCES := boot/exception.s

KOS_CC := gcc
KOS_CFLAGS := -static -nostdlib -EL -fno-tree-loop-distribute-patterns
KOS_CFLAGS += -march=r3000 -mtune=r3000 -mfp32 -ffreestanding -nostdinc
KOS_SOURCES := $(wildcard koalaos/*.c)
KOS_SOURCES += $(wildcard koalaos/libc/*.c)
KOS_SOURCES += $(wildcard koalaos/sys/*.c)
KOS_SOURCES += $(wildcard koalaos/usr/*.c)
KOS_SOURCES += $(wildcard koalaos/hw/*.c)

VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)
OS_INFO := $(shell uname -rmo)

SOURCES := $(wildcard *.c)
SOURCES += $(wildcard src/*.c)

bin/main main.c:
	mkdir -p bin

	$(CC) $(SOURCES) -o bin/koalaboard \
		-DVERSION_TAG="$(VERSION_TAG)" \
		-DCOMMIT_HASH="$(COMMIT_HASH)" \
		-DOS_INFO="$(OS_INFO)" \
		-Isrc -g $(CFLAGS)

koalaos:
	$(CROSS_PREFIX)-$(KOS_CC) \
		$(KOS_SOURCES) \
		-o bin/koalaos.elf \
		-Ikoalaos \
		-DVERSION_TAG="$(VERSION_TAG)" \
		-DCOMMIT_HASH="$(COMMIT_HASH)" \
		-DOS_INFO="$(OS_INFO)" \
		$(KOS_CFLAGS)

firmware:
	mkdir -p build
	mkdir -p bin

	$(CROSS_PREFIX)-as $(FW_SOURCES) -o build/boot.o -mips1 -EL
	$(CROSS_PREFIX)-ld build/boot.o -T boot/script.ld -o bin/firmware.bin

clean:
	rm -rf bin
	rm -rf build
