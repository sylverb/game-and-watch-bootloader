#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

extern uint32_t log_idx;
extern char logbuf[1024 * 4];

int _write(int file, char *ptr, int len)
{
    uint32_t idx = log_idx;
    if (idx + len + 1 > sizeof(logbuf))
    {
        idx = 0;
    }

    memcpy(&logbuf[idx], ptr, len);
    idx += len;
    logbuf[idx] = '\0';

    log_idx = idx;

    return len;
}
