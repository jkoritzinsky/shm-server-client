#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "stats.h"

sem_t *mutex;

void* shm_root;

stats_t* stats_init(key_t key)
{
   int id = shmget(key, 0, 0);
   if(id == -1)
     return NULL;
   stats_t* shm = shm_root = shmat(id, NULL, 0);
   if(shm == (void*)-1)
     return NULL;
   if(mutex == NULL)
   {
    if ((mutex = sem_open("shm_semaphore_537_p3a_crossDiscussion", 0, 0644, 1)) == SEM_FAILED) {
        perror("sem_open");
        exit(1);
      }
  }
  if(sem_wait(mutex) == -1)
  {
     perror("sem_wait");
     exit(1);
  }
  int i;
  for(i = 0; i < 16; ++i, ++shm){
    if(!shm->pid){
      shm->pid = (int)getpid();
      sem_post(mutex);
      return shm;
    }
  }
  sem_post(mutex);
  return NULL;
}

int stats_unlink(key_t key)
{
   if(mutex == NULL)
   {
    if ((mutex = sem_open("shm_semaphore_537_p3a_crossDiscussion", 0, 0644, 1)) == SEM_FAILED) {
        perror("sem_open");
        exit(1);
      }
  }
  if(sem_wait(mutex) == -1)
  {
     perror("sem_wait");
     exit(1);
  }
  int i;
  stats_t* shm = shm_root;
  for(i = 0; i < 16; ++i, ++shm){
    if(shm->pid == getpid()){
      shm->pid = 0;
      sem_post(mutex);
      return shmdt(shm_root);
    }
  }
  sem_post(mutex);
  return -1;
}
