#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <string.h>
#include "stats.h"

key_t key;
stats_t* stats;

void handle_exit();

void handle_signal(int sig)
{
  stats_unlink(key);
  exit(0);
}

void usage(char* exec)
{
  printf("Usage: %s -k key -p priority -s sleeptime_ns -c cputime_ns\n", exec);
}


int main(int argc, char*argv[])
{
  long sleeptime_ns = 1000;
  long cputime_ns = 1000000;
  char c;
  while ((c = getopt(argc, argv, "k:p:s:c:")) != -1) {
    switch (c) {
      case 'k':
        key = atoi(optarg);
        break;
      case 'p':
        if(setpriority(PRIO_PROCESS, getpid(), atoi(optarg)) == -1){
          perror("setpriority");
          return 1;
        }
        break;
      case 's':
        sleeptime_ns = atol(optarg);
        break;
      case 'c':
        cputime_ns = atol(optarg);
        break;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  stats = stats_init(key);
  if(!stats){
    fprintf(stderr, "Failed to attach client.\n");
    return 1;
  }
  signal(SIGINT, handle_signal);
  strncpy(stats->proc_name, argv[0], 15);
  stats->proc_name[15] = '\0';
  int i;
  for(i = 0; ; ++i){
    struct timespec sleep = {0, sleeptime_ns};
    nanosleep(&sleep, NULL);
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    struct timespec spin_till = current;
    spin_till.tv_nsec += cputime_ns;
    if(spin_till.tv_nsec / 1000000000 > 0){
      spin_till.tv_sec += spin_till.tv_nsec / 1000000000;
      spin_till.tv_nsec %= 1000000000;
    }
    while(1){
      clock_gettime(CLOCK_MONOTONIC, &current);
      if(spin_till.tv_sec == current.tv_sec && spin_till.tv_nsec <= current.tv_nsec)
        break;
      if(spin_till.tv_sec < current.tv_sec)
        break;
    }
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, &r_usage);
    double cpu_secs = (r_usage.ru_utime.tv_sec + r_usage.ru_stime.tv_sec) + (r_usage.ru_utime.tv_usec + r_usage.ru_stime.tv_usec) / 10e-6;
    stats->counter = i;
    stats->priority = getpriority(PRIO_PROCESS, getpid());
    stats->cpu_secs = cpu_secs;
  }
}
