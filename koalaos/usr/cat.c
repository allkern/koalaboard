#include "libc/string.h"
#include "libc/stdio.h"
#include "sys/fs.h"

#include "shell.h"

int usr_cat(int argc, const char* argv[]) {
    char* cwd = shell_get_cwd();

    const char* path;

    if (!argv[1]) {
        printf("Path \'%s\' does not point to a file\n", cwd);

        return 1;
    } else {
        path = argv[1];

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

            path = buf;
        }

        // else absolute path
    }

    char buf[512];

    struct file_s file;
    uint32_t status;

    if (fat_file_open(&file, path, strlen(path))) {
        printf("Could not open file \'%s\'\n", path);

        return 1;
    }

    if (fat_file_read(&file, buf, file.size, &status)) {
        printf("Could not read file \'%s\'\n", path);

        return 1;
    }

    puts(buf);

    return EXIT_SUCCESS;
}