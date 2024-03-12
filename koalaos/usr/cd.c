#include "libc/string.h"
#include "libc/stdio.h"
#include "sys/fs.h"

#include "shell.h"

int usr_cd(int argc, const char* argv[]) {
    if (!argv[1])
        return EXIT_SUCCESS;

    if (!strncmp(argv[1], ".", 1))
        return EXIT_SUCCESS;

    const char* path;

    path = argv[1];

    if (!strncmp(argv[1], "..", 2)) {
        struct dir_s dir;

        fstatus fstat = fat_dir_open(&dir, path, strlen(path));

        if (fstat) {
            printf("Could not change current directory to \'%s\' (%u)\n",
                path,
                fstat
            );

            return EXIT_FAILURE;
        }

        fat_dir_close(&dir);
    }

    char* cwd = shell_get_cwd();

    if (path[1] != ':') {
        char buf[128];

        char* p = cwd;
        char* q = buf;
        char* r = path;

        while (*p != '\0')
            *q++ = *p++;

        while (*r != '\0')
            *q++ = *r++;

        *q++ = '/';
        *q = '\0';

        path = buf;
    }

    struct dir_s dir;
    struct info_s info;

    fstatus fstat_open = fat_dir_open(&dir, path, strlen(path));
    fstatus fstat_read = fat_dir_read(&dir, &info);

    fat_dir_close(&dir);

    printf("attribute=%02x size=%08x\n",
        info.attribute,
        info.size
    );

    if (fstat_open || fstat_read || !(info.attribute & ATTR_DIR)) {
        printf("Could not change current directory to \'%s\' (%u, %u)\n",
            path,
            fstat_open,
            fstat_read
        );

        return EXIT_FAILURE;
    }

    strncpy(cwd, path, 512);

    return EXIT_SUCCESS;
}