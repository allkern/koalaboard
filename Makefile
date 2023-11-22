.PHONY: koalaos

.ONESHELL:

CC := gcc
CFLAGS := -g -DLOG_USE_COLOR -Ofast

KOS_CC := mipsel-linux-gnu-gcc
KOS_CFLAGS := -static -nostdlib -EL -Ofast
KOS_CFLAGS += -march=r3000 -mtune=r3000 -mfp32

VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)
OS_INFO := $(shell uname -rmo)

SOURCES := main.c
SOURCES += $(wildcard src/*.c)

bin/main frontend/main.c:
	mkdir -p bin

	$(CC) $(SOURCES) -o bin/main \
		-Isrc -g

koalaos:
	mkdir -p bin

	$(KOS_CC) koalaos/*.c -o bin/koalaos.elf \
		-Ikoalaos \
		$(KOS_CFLAGS)

clean:
	rm -rf "bin"
