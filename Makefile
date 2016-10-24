.PHONY: all run_server run_client clean debug_client

CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -g

all: stats_client stats_server

stats_client: libstats.so stats_client.c
	gcc stats_client.c -o stats_client $(CFLAGS) -lpthread -lstats -L.

stats_server: stats_server.c
	gcc stats_server.c -o stats_server $(CFLAGS) -lpthread

libstats.so: stats.o
	gcc -shared -o libstats.so stats.o

stats.o: stats.c
	gcc -c -fpic stats.c $(CFLAGS)

run_server: stats_server
	stats_server -k $(KEY)

run_client: stats_client
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) stats_client -k $(KEY) -p $(PRIORITY) -s $(SLEEPTIME) -c $(CPUTIME) 
	
debug_client: stats_client
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) gdb stats_client 
  
clean: all
	rm stats_client stats_server libstats.so *.o
