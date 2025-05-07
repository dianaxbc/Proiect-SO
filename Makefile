CC=gcc
CFLAGS=-Wall

all: treasure_manager treasure_hub treasure_monitor

treasure_manager: main.o treasure_manager.o
	$(CC) $(CFLAGS) -o treasure_manager main.o treasure_manager.o

main.o: main.c treasure_manager.h
	$(CC) $(CFLAGS) -c main.c

treasure_manager.o: treasure_manager.c treasure_manager.h
	$(CC) $(CFLAGS) -c treasure_manager.c

treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) -o treasure_hub treasure_hub.c

treasure_monitor: treasure_monitor.c
	$(CC) $(CFLAGS) -o treasure_monitor treasure_monitor.c

clean:
	rm -f *.o treasure_manager treasure_hub treasure_monitor
