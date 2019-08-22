#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdbool.h>

#define BUFFLEN 256
#define TIME_STAMP_LEN 25

void* obj_malloc(size_t size);
char *gen_time_stamp(); // generate time stamp in format: aaaa/mm/dd_hh:mm:ss
void usage(char *usg);

#endif
