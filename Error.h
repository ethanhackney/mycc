#ifndef ERROR_H
#define ERROR_H

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// @fmt:        format string
// @...:        variadic list
extern void usage(const char *fmt, ...);

// @fmt:        format string
// @...:        variadic list
extern void error(const char *fmt, ...);

#endif
