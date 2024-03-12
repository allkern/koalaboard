#include "sys/fs.h"
#include "usr/shell.h"

#include "libc/string.h"
#include "libc/stdlib.h"
#include "libc/stdio.h"

char dechex[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

int usr_dir(int argc, const char* argv[]) {
    char* cwd = shell_get_cwd();

    const char* path;

    if (!argv[1]) {
        path = cwd;
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

    current_volume = volume_get_first();

    char serial[10];
    char* ptr = serial;

    for (int i = 0; i < 8; i++) {
        if (i == 4)
            *ptr++ = '-';

        *ptr++ = dechex[(current_volume->serial >> ((7 - i) * 4)) & 0xf];
    }

    serial[9] = '\0';

    printf(" Volume in drive %c is %s\n",
        current_volume->letter,
        current_volume->label
    );

    printf(" Volume Serial Number is %s\n", serial);

    printf("\n Directory of %s\n\n", path);

    struct dir_s dir;
    struct info_s info;

    fstatus fstat_open = fat_dir_open(&dir, path, strlen(path));
    fstatus fstat_read = fat_dir_read(&dir, &info);

    fat_dir_close(&dir);

    // printf("attribute=%02x size=%08x\n",
    //     info.attribute,
    //     info.size
    // );

    if (fstat_open || fstat_read) {
        printf("File not found (%u, %u)\n",
            fstat_open,
            fstat_read
        );

        return EXIT_FAILURE;
    }

    if ((!(info.attribute & ATTR_DIR)) && !(info.attribute & ATTR_VOL_LABEL)) {
        printf("Path \'%s\' does not refer to a directory", path);

        return EXIT_FAILURE;
    }

    fat_dir_open(&dir, path, strlen(path));

	fstatus status;

	do {
		status = fat_dir_read(&dir, &info);

		// Print the information
		if (status == FSTATUS_OK)
			fat_print_info(&info);
	} while (status != FSTATUS_EOF);

    fat_dir_close(&dir);
}