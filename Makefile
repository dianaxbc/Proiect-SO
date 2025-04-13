CC = gcc
CFLAGS = -Wall

all: treasure_manager

treasure_manager: main.o treasure_manager.o
	$(CC) -o treasure_manager main.o treasure_manager.o

main.o: main.c treasure_manager.h
	$(CC) $(CFLAGS) -c main.c

treasure_manager.o: treasure_manager.c treasure_manager.h
	$(CC) $(CFLAGS) -c treasure_manager.c

clean:
	rm -f *.o treasure_manager
