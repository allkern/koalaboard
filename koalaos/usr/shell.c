#include "libc/string.h"
#include "libc/stdio.h"
#include "sys/fs.h"
#include "hw/gpu.h"

#include "shell.h"
#include "dir.h"
#include "test.h"
#include "cd.h"
#include "help.h"
#include "cat.h"
#include "color.h"
#include "clear.h"
#include "print.h"
#include "chip8.h"

void HACK_malloc_argv() {
    char* base = HACK_MALLOC_BASE;

    for (int i = 0; i < MAX_ARGS; i++) {
        __argv[i] = base;

        base += MAX_ARGLEN;
    }
}

void HACK_argv_fix(int argc) {
    for (int i = argc; i < MAX_ARGS; i++)
        __argv[i] = NULL;
}

void usr_shell_init(void) {
    struct volume_s* vol = volume_get_first();

    // <letter>:/
    __envp.cwd[0] = vol->letter;
    __envp.cwd[1] = ':';
    __envp.cwd[2] = '/';
    __envp.cwd[3] = '\0';

    SEF_REGISTER(cat);
    SEF_REGISTER(cd);
    SEF_REGISTER(chip8);
    SEF_REGISTER(clear);
    SEF_REGISTER(color);
    SEF_REGISTER(dir);
    SEF_REGISTER(help);
    SEF_REGISTER(print);
    SEF_REGISTER(test);

    // Huge hack, we don't use malloc that much anyways
    HACK_MALLOC_BASE = malloc(0);

    // Allocate two contiguous blocks
    void* dummy = malloc(0);
}

void usr_shell_register(sef_proto fn, const char* name, const char* desc) {
    sef[sef_index].fn = fn;
    sef[sef_index].name = name;
    sef[sef_index++].desc = desc;
}

int getchar_block() {
    char c = getchar();

    while (c == -1)
        c = getchar();

    return c;
}

void init_argv(char* buf) {
    __argc = 0;

    char* ptr = buf;

    while (*ptr != '\0') {
        if (*ptr == '\"') {
            char* arg = __argv[__argc++];

            ptr++;

            while ((*ptr != '\"') && (*ptr != '\0'))
                *arg++ = *ptr++;

            ptr++;

            *arg = '\0';
        } else if (*ptr == ' ') {
            ptr++;
        } else {
            char* arg = __argv[__argc++];

            while ((*ptr != ' ') && (*ptr != '\0'))
                *arg++ = *ptr++;

            *arg = '\0';
        }
    }
}

int shell_exec(const char* buf) {
    __argc = 0;

    for (int i = 0; i < MAX_SEF_COUNT; i++) {
        if (!sef[i].name)
            continue;

        size_t len = strlen(sef[i].name);

        if (!strncmp(sef[i].name, buf, len)) {
            HACK_malloc_argv();
            init_argv(buf);
            HACK_argv_fix(__argc);

            // To-do: Do something with return value
            return sef[i].fn(__argc, (const char**)__argv);
        }
    }

    printf("Unknown command \'%s\'\n", buf);
}

void usr_shell() {
    char* ptr = cmd;

    while (1) {
        printf("\r%s> %s", __envp.cwd, cmd);

        char c = getchar_block();

        switch (c) {
            // Enter key pressed
            case '\r': {
                if (cmd[0] == '\0') {
                    putchar('\n');

                    goto done;
                }

                putchar('\n');

                *ptr = '\0';

                shell_exec(cmd);

                done:

                // Reset command
                cmd[0] = '\0';

                ptr = cmd;
            } break;

            case '\b': {
                if (ptr - cmd) {
                    gpu_set_xpos(gpu_get_xpos() - 8);
                    gpu_putchar(' ');

                    *(--ptr) = '\0';
                }
            } break;

            default: {
                *ptr++ = c;
                *ptr = '\0';
            } break;
        }
    }

    printf("Exit shell\n");

    while(1);
}

char* shell_get_cwd(void) {
    return __envp.cwd;
}

struct sef_desc* shell_get_sef_desc(int i) {
    return &sef[i];
}

void shell_get_absolute_path(char* path, char* buf, size_t size) {
    char* cwd = __envp.cwd;

    const char* abs = path;
    
    if (path[1] != ':') {
        char buf[128];

        char* p = cwd;
        char* q = buf;
        char* r = path;

        while (*p != '\0')
            *q++ = *p++;

        while (*r != '\0')
            *q++ = *r++;

        *q = '\0';

        abs = buf;
    }

    strncpy(abs, buf, size);
}