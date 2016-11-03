#include "stats.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

key_t key;
stats_t* stats;

void handle_signal(int sig) {
  stats_unlink(key);
  if (mutex != NULL) {
    sem_close(mutex);
    mutex = NULL;
  }
  exit(0);
}

void usage(char* exec) {
  printf("Usage: %s -k key -p priority -s sleeptime_ns -c cputime_ns\n", exec);
}

int main(int argc, char* argv[]) {
  long sleeptime_ns = 1000;
  long cputime_ns = 1000000;
  int priority = 1;
  char c;
  while ((c = getopt(argc, argv, "k:p:s:c:")) != -1) {
    switch (c) {
      case 'k':
        key = atoi(optarg);
        break;
      case 'p':
        if (setpriority(PRIO_PROCESS, getpid(), atoi(optarg)) == -1) {
          perror("setpriority");
          return 1;
        }
        priority = getpriority(PRIO_PROCESS, getpid());
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
  if (!stats) {
    fprintf(stderr, "Failed to attach client.\n");
    exit(1);
  }
  signal(SIGINT, handle_signal);
  strncpy(stats->proc_name, argv[0], 15);
  stats->proc_name[15] = '\0';
  stats->priority = priority;
  double cpu_secs = 0;
  for (stats->counter = 0;; stats->counter++) {
    struct timespec sleep = {0, sleeptime_ns};
    nanosleep(&sleep, NULL);

    struct timespec current;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &current);
    struct timespec spin_start = current, spin_till = current;

    spin_till.tv_nsec += cputime_ns;
    if (spin_till.tv_nsec / 1e9 > 0) {
      spin_till.tv_sec += spin_till.tv_nsec / 1e9;
      spin_till.tv_nsec %= (long)1e9;
    }

    while (1) {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &current);
      if (spin_till.tv_sec == current.tv_sec &&
          spin_till.tv_nsec <= current.tv_nsec)
        break;
      if (spin_till.tv_sec < current.tv_sec) break;
    }
    cpu_secs += (current.tv_sec - spin_start.tv_sec)
      + (current.tv_nsec - spin_start.tv_nsec) * 1e-9;
    stats->cpu_secs = cpu_secs;
  }
}
