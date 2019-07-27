#include "../header/hash.h"

hash_t calculate_hash(char s[BUFFLEN])
{
  hash_t hash = (hash_t)obj_malloc(SHA256_DIGEST_LENGTH);
  SHA256((hash_t)s, strlen(s), hash);
  return hash;
}


void print_hash(hash_t h)
{
  // printf("\n");
  for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    printf("%02x", h[i]);
  }
  printf("\n");
}


void hashcpy(hash_t dst, hash_t src)
{
  strncpy((char*)dst, (const char *restrict)src, SHA256_DIGEST_LENGTH);
}


bool hash_equal(hash_t a, hash_t b)
{
  if(a == NULL || b == NULL)
    printf("\nhash pointer of a or b is null\n");

  return (strncmp((const char *)a, (const char *)b, SHA256_DIGEST_LENGTH) == 0) ? true : false;
}
