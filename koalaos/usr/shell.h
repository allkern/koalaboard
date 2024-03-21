#ifndef SHELL_H
#define SHELL_H

// Shell environment
static struct sh_envp {
    char cwd[512];
} __envp;

// Shell embedded functions (help, dir, etc.)
typedef int (*sef_proto)(int, const char* argv[]);

#define MAX_SEF_COUNT 32

static unsigned int sef_index = 0;

static struct sef_desc {
    const char* name;
    const char* desc;
    sef_proto fn;
} sef[MAX_SEF_COUNT];

// CLI
#define MAX_ARGS 16
#define MAX_ARGLEN 128

static int __argc;
static char* __argv[MAX_ARGS];

#define MAX_CMD 256

static char prev[MAX_CMD];
static char curr[MAX_CMD];
static char* cmd;

void usr_shell_init(void);
void usr_shell_register(sef_proto fn, const char* name, const char* desc);
void usr_shell(void);
int shell_exec(const char* args);
char* shell_get_cwd(void);
struct sef_desc* shell_get_sef_desc(int i);
char* shell_get_path(char* path);

static char* HACK_MALLOC_BASE;

#endif