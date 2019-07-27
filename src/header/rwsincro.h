#ifndef RWSINCRO_H
#define RWSINCRO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <retibtc/utils.h>

enum priority{ R_PRIO, W_PRIO };
enum roles{ READER, WRITER };

struct s_rw_sincro
{
  pthread_mutex_t x;
  int n_read;
  pthread_mutex_t y;
  int n_write;
  pthread_mutex_t z;
  sem_t *wsem;
  sem_t *rsem;
  short prio;
};

typedef struct s_rw_sincro *RW_sincro;
#define RWSINCRO sizeof(struct s_rw_sincro)

RW_sincro rw_sincro_create(short prio);
void rw_sincro_destroy(RW_sincro rw_stuff);

void rw_sincro_entry_section(RW_sincro rw_stuff, short role);
void rw_sincro_exit_section(RW_sincro rw_stuff, short role);


// W_PRIO && READER
void wr_entry_section(RW_sincro rw_stuff);

// W_PRIO && READER
void wr_exit_section(RW_sincro rw_stuff);


// W_PRIO && WRITER
void ww_entry_section(RW_sincro rw_stuff);

// W_PRIO && WRITER
void ww_exit_section(RW_sincro rw_stuff);


// R_PRIO && READER ENTRY
void rr_entry_section(RW_sincro rw_stuff);

// R_PRIO && READER EXIT
void rr_exit_section(RW_sincro rw_stuff);


// R_PRIO && WRITER ENTRY
void rw_entry_section(RW_sincro rw_stuff);

// R_PRIO && WRITER EXIT
void rw_exit_section(RW_sincro rw_stuff);


#endif
