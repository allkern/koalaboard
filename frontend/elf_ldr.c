#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "elf_ldr.h"

#define ELF_ERROR(cond, ...) \
    if (cond) {              \
        printf(__VA_ARGS__); \
        return 1;            \
    }

elf_file_t* elf_create() {
    return (elf_file_t*)malloc(sizeof(elf_file_t));
}

int elf_load(elf_file_t* elf, const char* path) {
    elf->file = fopen(path, "rb");

    ELF_ERROR(
        !elf->file,
        "Could not open file \'%s\'\n", path
    );

    Elf32_Ehdr* ehdr = malloc(sizeof(Elf32_Ehdr));

    fread(ehdr, 1, sizeof(Elf32_Ehdr), elf->file);

    char buf[] = { '\x7f', 'E', 'L', 'F' };

    ELF_ERROR(
        strncmp(ehdr->e_ident, buf, 3),
        "File does not contain magic ELF string\n"
    );

    Elf32_Phdr** phdr = malloc(ehdr->e_phnum * sizeof(Elf32_Phdr*));

    for (int i = 0; i < ehdr->e_phnum; i++) {
        phdr[i] = malloc(ehdr->e_phentsize);

        fseek(elf->file, ehdr->e_phoff + (i * ehdr->e_phentsize), SEEK_SET);
        fread(phdr[i], 1, ehdr->e_phentsize, elf->file);
    }

    elf->ehdr = ehdr;
    elf->phdr = phdr;

    return 0;
}

void elf_load_segment(elf_file_t* elf, int i, void* buf) {
    if (elf->phdr[i]->p_type != PT_LOAD) {
        printf("Program header segment is not loadable\n");

        return;
    }

    fseek(elf->file, elf->phdr[i]->p_offset, SEEK_SET);
    fread(buf, 1, elf->phdr[i]->p_filesz, elf->file);
}

void elf_destroy(elf_file_t* elf) {
    fclose(elf->file);
    free(elf->ehdr);
    free(elf);
}