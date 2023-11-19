#include "elf.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ELF_ERROR(cond, ...) \
    if (cond) {              \
        printf(__VA_ARGS__); \
        return 1;            \
    }

elf_file_t* elf_create() {
    return (elf_file_t*)malloc(sizeof(elf_file_t));
}

int elf_load(elf_file_t* elf, const char* path) {
    FILE* file = fopen(path, "rb");

    ELF_ERROR(
        !file,
        "Could not open file \'%s\'\n", path
    );

    Elf32_Ehdr* ehdr = malloc(sizeof(Elf32_Ehdr));

    fread(ehdr, 1, sizeof(Elf32_Ehdr), file);

    ELF_ERROR(
        strncmp(ehdr->e_ident, "ELF", 3),
        "File does not contain magic ELF string\n"
    );

    Elf32_Phdr** phdr = malloc(ehdr->e_phnum * sizeof(Elf32_Phdr*));

    for (int i = 0; i < ehdr->e_phnum; i++) {
        phdr[i] = malloc(ehdr->e_phentsize);

        fseek(file, ehdr->e_phoff + (i * ehdr->e_phentsize), SEEK_SET);
        fread(phdr[i], 1, ehdr->e_phentsize, file);
    }

    fclose(file);

    return 0;
}

void elf_destroy(elf_file_t* elf) {
    free(elf->ehdr);
    free(elf);
}