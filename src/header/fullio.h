#ifndef FULLIO_H
#define FULLIO_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

ssize_t full_read(int fd, void *buf, size_t count);
ssize_t full_write(int fd, const void *buf, size_t count);

#endif
