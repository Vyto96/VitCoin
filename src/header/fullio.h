#ifndef FULLIO_H
#define FULLIO_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

// returns the number of unread bytes
ssize_t full_read(int fd, void *buf, size_t count);
// returns the number of unwritten bytes
ssize_t full_write(int fd, const void *buf, size_t count);

#endif
