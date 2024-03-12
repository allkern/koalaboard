#ifndef UTIL_H
#define UTIL_H

#define SEF_REGISTER(cmd) \
    usr_shell_register(usr_ ## cmd, USR_NAME_ ## cmd, USR_DESC_ ## cmd)

#define SEF_DEFINE(cmd, desc) \
    int usr_ ## cmd(int argc, const char* argv[]); \
    static const char* USR_NAME_ ##cmd = #cmd; \
    static const char* USR_DESC_ ##cmd = desc;

#endif