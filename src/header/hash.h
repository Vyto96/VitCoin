#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h" // for obj_malloc & BUFFLEN def

typedef unsigned char* hash_t;

// calculate hash for the string provided
hash_t calculate_hash(char s[BUFFLEN]);

// hexadecimal hash print
void print_hash(hash_t h);

// strcpy wrapper for assign hash to another
void hashcpy(hash_t dst, hash_t src);

// strncmp wrapper for compare hash with another
bool hash_equal(hash_t dst, hash_t src);

#endif
