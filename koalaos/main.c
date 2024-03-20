#include "libc/string.h"
#include "libc/stdio.h"
#include "util/mmio.h"
#include "usr/dir.h"
#include "hw/uart.h"
#include "hw/vmc.h"
#include "hw/gpu.h"
#include "config.h"
#include "c8.h"

#include "usr/shell.h"

int main() {
    puts("Welcome to KoalaOS!\n\nCopyright (C) 2024 Allkern/Lisandro Alarcon\n");

    usr_shell_init();
    usr_shell();

    return 0;
}