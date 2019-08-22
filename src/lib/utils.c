#include "../header/utils.h"


void *obj_malloc(size_t size)
{
  void *ptr;
  ptr  = malloc(size);
  memset(ptr, 0, size);
  if(ptr == NULL)
  {
    perror("Failed to allocate object");
    exit(EXIT_FAILURE);
  }
  return ptr;
}


char *gen_time_stamp()
{
  char *time_stamp = (char*)obj_malloc(sizeof(char) * TIME_STAMP_LEN);

  time_t timeval = time(NULL);
  struct tm *t = gmtime(&timeval);


  // if daylight saving time is in effect
  if (t->tm_isdst == 0)
    sprintf(time_stamp, "%d/%d/%d_%d:%d:%d", \
      t->tm_year + 1900,
      t->tm_mon + 1,  // month (0-11)
      t->tm_mday, \
      // +1 for daylight saving time and +1 for the italian time zone
      t->tm_hour + 2, \
      t->tm_min, t->tm_sec);
  else
    sprintf(time_stamp, "%d/%d/%d_%d:%d:%d", \
      t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, \
      // +1 for the italian time zone
      t->tm_hour + 1, \
      t->tm_min, t->tm_sec);

  return time_stamp;
}


void usage(char *usg)
{
  fprintf(stderr, usg);
  exit(EXIT_FAILURE);
}
