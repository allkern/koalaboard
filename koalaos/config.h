#ifndef CONFIG_H
#define CONFIG_H

#ifdef _MSC_VER
#define __COMPILER__ "msvc"
#elif __GNUC__
#define __COMPILER__ "gcc"
#endif

#ifdef __mips__
#define __ARCH__ "mips"
#else
#define __ARCH__ "unknown"
#endif

#ifndef OS_INFO
#define OS_INFO unknown
#endif
#ifndef VERSION_TAG
#define VERSION_TAG latest
#endif
#ifndef COMMIT_HASH
#define COMMIT_HASH latest
#endif

#define STR1(m) #m
#define STR(m) STR1(m)

#define TERM_UART

#ifdef TERM_GPU
#define TERM_FUNC gpu_putchar
#elif defined(TERM_UART)
#define TERM_FUNC uart_send_byte
#else
#define TERM_FUNC gpu_putchar
#endif

#endif