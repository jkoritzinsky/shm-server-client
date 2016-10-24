.PHONY: all run_server run_client

CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -g

all: stat_client stat_server

stat_client: libstats.so stat_client.c
	gcc stat_client.c -o stat_client $(CFLAGS) -lstats -L.

stat_server: stat_server.c
	gcc stat_server.c -o stat_server $(CFLAGS) -lpthread

libstats.so: stats.o
	gcc -shared -o libstats.so stats.o

stats.o: stats.c
	gcc -c -fpic stats.c $(CFLAGS) -lpthread

run_server: stat_server
	stat_server -k $(KEY)

run_client: stat_client
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) stat_client -k $(KEY) -p $(PRIORITY) -s $(SLEEPTIME) -c $(CPUTIME) 

