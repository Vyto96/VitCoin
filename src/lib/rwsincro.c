#include <retibtc/rwsincro.h>


RW_sincro rw_sincro_create(short prio)
{
  RW_sincro ptr_rw_stuff = (RW_sincro)obj_malloc(RWSINCRO);

  sem_t *rsem = (sem_t*)obj_malloc(sizeof(sem_t));
  sem_t *wsem = (sem_t*)obj_malloc(sizeof(sem_t));
  sem_init(rsem, 0, 1);
  sem_init(wsem, 0, 1);

  struct s_rw_sincro rw_stuff = {
    PTHREAD_MUTEX_INITIALIZER,    // x
    0,                            // n_read
    PTHREAD_MUTEX_INITIALIZER,    // y
    0,                            // n_write
    PTHREAD_MUTEX_INITIALIZER,    // z
    wsem,
    rsem,
    prio
  };

  *ptr_rw_stuff = rw_stuff;
  return ptr_rw_stuff;

}

void rw_sincro_destroy(RW_sincro rw_stuff)
{
  sem_destroy(rw_stuff->wsem);
  sem_destroy(rw_stuff->rsem);
  free(rw_stuff);
}


/*PRIO & TYPE SELECTOR************************************/
void rw_sincro_entry_section(RW_sincro rw_stuff, short role)
{
  if(rw_stuff->prio == R_PRIO)
    if(role == READER)
      rr_entry_section(rw_stuff); // R_PRIO && READER
    else
      rw_entry_section(rw_stuff); // R_PRIO && WRITER
  else
    if(role == READER)
      wr_entry_section(rw_stuff); // W_PRIO && READER
    else
      ww_entry_section(rw_stuff); // W_PRIO && WRITER
}

void rw_sincro_exit_section(RW_sincro rw_stuff, short role)
{
  if(rw_stuff->prio == R_PRIO)
    if(role == READER)
      rr_exit_section(rw_stuff); // R_PRIO && READER
    else
      rw_exit_section(rw_stuff); // R_PRIO && WRITER
  else
    if(role == READER)
      wr_exit_section(rw_stuff); // W_PRIO && READER
    else
      ww_exit_section(rw_stuff); // W_PRIO && WRITER

}

/*W_PRIO***************************************************/
/*READER*/
void wr_entry_section(RW_sincro rw_stuff)
{
  pthread_mutex_lock(&(rw_stuff->z));
    sem_wait(rw_stuff->rsem);
      pthread_mutex_lock(&(rw_stuff->x));

        rw_stuff->n_read++;
        if(rw_stuff->n_read == 1)
          sem_wait(rw_stuff->wsem);

      pthread_mutex_unlock(&(rw_stuff->x));
    sem_post(rw_stuff->rsem);
  pthread_mutex_unlock(&(rw_stuff->z));
}
/*
  Reader Critic Section
*/
/*READER*/
void wr_exit_section(RW_sincro rw_stuff)
{
  pthread_mutex_lock(&(rw_stuff->x));

    rw_stuff->n_read--;
    if(rw_stuff->n_read == 0)
      sem_post(rw_stuff->wsem);

  pthread_mutex_unlock(&(rw_stuff->x));
}


/*WRITER*/
void ww_entry_section(RW_sincro rw_stuff)
{
  pthread_mutex_lock(&(rw_stuff->y));

    rw_stuff->n_write++;
    if(rw_stuff->n_write == 1)
      sem_wait(rw_stuff->rsem);

  pthread_mutex_unlock(&(rw_stuff->y));

  sem_wait(rw_stuff->wsem);
}
/*
  Writer Critic Section
*/
/*WRITER*/
void ww_exit_section(RW_sincro rw_stuff)
{
  sem_post(rw_stuff->wsem);

  pthread_mutex_lock(&(rw_stuff->y));

    rw_stuff->n_write--;
    if(rw_stuff->n_write == 0)
      sem_post(rw_stuff->rsem);

  pthread_mutex_unlock(&(rw_stuff->y));
}


/*R_PRIO***************************************************/
/*READER */
void rr_entry_section(RW_sincro rw_stuff)
{
  pthread_mutex_lock(&(rw_stuff->x));

    rw_stuff->n_read++;
    if(rw_stuff->n_read == 1)
      sem_wait(rw_stuff->wsem);

  pthread_mutex_unlock(&(rw_stuff->x));
}
/*
  Reader Critic Section
*/
/*READER*/
void rr_exit_section(RW_sincro rw_stuff)
{
  //same instructions
  wr_exit_section(rw_stuff);
}


/*WRITER*/
void rw_entry_section(RW_sincro rw_stuff)
{
  sem_wait(rw_stuff->wsem);
}
/*
  Writer Critic Section
*/
/*WRITER*/
void rw_exit_section(RW_sincro rw_stuff)
{
  sem_post(rw_stuff->wsem);
}
