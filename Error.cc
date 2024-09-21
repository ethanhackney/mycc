#include "Error.h"

void usage(const char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        fprintf(stderr, ": %s\n", strerror(EINVAL));
        exit(EXIT_FAILURE);
}

void error(const char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        fprintf(stderr, ": %s\n", strerror(errno));
        exit(EXIT_FAILURE);
}
