#ifndef RWSINCRO_H
#define RWSINCRO_H

/*
  Implementation of algorithms for the synchronization of writers and readers,
   with choice of priorities, taken from the book:
   "Operating systems. Concepts and examples" by A. Silberschatz.
*/


#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "utils.h"

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


// W_PRIO && READER ENTRY SECTION
void wr_entry_section(RW_sincro rw_stuff);

// W_PRIO && READER EXIT SECTION
void wr_exit_section(RW_sincro rw_stuff);


// W_PRIO && WRITER ENTRY SECTION
void ww_entry_section(RW_sincro rw_stuff);

// W_PRIO && WRITER EXIT SECTION
void ww_exit_section(RW_sincro rw_stuff);


// R_PRIO && READER ENTRY SECTION
void rr_entry_section(RW_sincro rw_stuff);

// R_PRIO && READER EXIT SECTION
void rr_exit_section(RW_sincro rw_stuff);


// R_PRIO && WRITER ENTRY SECTION
void rw_entry_section(RW_sincro rw_stuff);

// R_PRIO && WRITER EXIT SECTION
void rw_exit_section(RW_sincro rw_stuff);


#endif
