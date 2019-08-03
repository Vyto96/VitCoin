#include "../header/fullio.h"

ssize_t full_read(int fd, void *buff, size_t count)
{
  size_t nleft;
  ssize_t nread;
  char * buf = (char*) buff;
  nleft = count;
  while (nleft > 0)
  {  /* repeat until no left */
    if( (nread = read(fd, buf, nleft)) < 0)
    {
      if (errno == EINTR) /* if interrupted by system call */
	      continue; /* repeat the loop */
      /*if errno is not EINTR, print errno and return negative nread */
      perror("full_read");
      return nread;

    }
    else
      if(nread == 0) /* if EOF */
        break; /* break loop here */
    /*if you are here, you have actually read bytes*/
    nleft -= nread; /* set left to read */
    buf += nread; /* set pointer */
  }
  return nleft;
}



ssize_t full_write(int fd, const void *buf, size_t count)
{
  size_t nleft;
  ssize_t nwritten;

  nleft = count;
  while (nleft > 0)
  { /*repeat until no left */
    if( (nwritten = write(fd, buf, nleft)) < 0)
    {
      if(errno == EINTR)
      { /* if interrupted by system call */
	      continue; /* repeat the loop */
      }
      /*if errno is not EINTR, print errno and return negative nwritten */
      perror("full_write");
      return nwritten;
    }
    /*if you are here, you have actually write bytes*/
    buf = (char*)buf + nwritten; /* set pointer */
    nleft -= nwritten;/* set left to write */
  }
  return nleft;
}
