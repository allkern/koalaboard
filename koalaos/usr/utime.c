#include "libc/string.h"
#include "libc/stdio.h"
#include "libc/time.h"
#include "libc/tzfile.h"

#include "hw/rtc.h"

int usr_time(int argc, const char* argv[]) {
    time_t t = time(NULL);

    const char* tz = argv[1];
    int tz_utchoff = 0;

    if (tz) {
        if (!strncmp(tz, "AR", 2)) tz_utchoff = -3;
    }

    t += tz_utchoff * SECS_PER_HOUR;

    struct tm* tm = gmtime(&t);
    
    printf("%s (%s)\n", asctime(tm), __secs_to_hrt(t));

    return EXIT_SUCCESS;
}