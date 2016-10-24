#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "stats.h"

key_t key;
int id;
sem_t* mutex;

void handle_exit();

void handle_signal(int sig)
{
  shmctl(id, IPC_RMID, NULL);
  sem_unlink("shm_semaphore_537_p3a_crossDiscussion");
  exit(0);
}

void usage(char* exec)
{
  printf("Usage: %s -k key\n", exec);
}

int main(int argc, char*argv[])
{
  char c;
  while ((c = getopt(argc, argv, "k:")) != -1) {
    switch (c) {
      case 'k':
        key = atoi(optarg);
        break;
      default:
        usage(argv[0]);
        return 1;
    }
  }
   if(mutex == NULL)
   {
      if ((mutex = sem_open("shm_semaphore_537_p3a_crossDiscussion", O_CREAT, 0644, 1)) == SEM_FAILED) {
          perror("sem_open");
          exit(1);
        }
  }

  id = shmget(key, sysconf(_SC_PAGESIZE), IPC_CREAT | IPC_EXCL | 0660);
  if(id == -1){
    perror("shmget");
    exit(1);
  }
  signal(SIGINT, handle_signal);
  
  stats_t* shm = shmat(id, NULL, 0);
  if(shm == (void*)-1){
    perror("shmat");
    exit(1);
  }

  stats_t volatile* stat;
  int i = 0, j = 0;
  for(;;++i){
    sleep(1);
    for(stat = shm, j=0; j<16; ++j, ++stat){
      if(stat->pid){
        printf("%d %d %s %d %.2f %d\n", i, stat->pid, stat->proc_name, stat->counter, stat->cpu_secs, stat->priority);
      }
    }
  }
}
