.PHONY: koalaos

.ONESHELL:

CC := gcc
CFLAGS := -g -DLOG_USE_COLOR -Ofast -lSDL2 -lSDL2main

KOS_CC := mipsel-linux-gnu-gcc
KOS_CFLAGS := -static -nostdlib -EL -fno-tree-loop-distribute-patterns
KOS_CFLAGS += -march=r3000 -mtune=r3000 -mfp32

VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)
OS_INFO := $(shell uname -rmo)

SOURCES := main.c
SOURCES += screen.c
SOURCES += $(wildcard src/*.c)

bin/main main.c:
	mkdir -p bin

	$(CC) $(SOURCES) -o bin/main \
		-DVERSION_TAG="$(VERSION_TAG)" \
		-DCOMMIT_HASH="$(COMMIT_HASH)" \
		-DOS_INFO="$(OS_INFO)" \
		-Isrc -g $(CFLAGS)

koalaos:
	$(KOS_CC) koalaos/*.c -o koalaos.elf \
		-Ikoalaos \
		-DVERSION_TAG="$(VERSION_TAG)" \
		-DCOMMIT_HASH="$(COMMIT_HASH)" \
		-DOS_INFO="$(OS_INFO)" \
		$(KOS_CFLAGS)

clean:
	rm -rf "bin"
