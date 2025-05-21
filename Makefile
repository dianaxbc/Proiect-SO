CC = gcc
CFLAGS = -Wall -g

all: treasure_manager treasure_monitor treasure_hub calculate_score

treasure_manager: treasure_manager.c
	$(CC) $(CFLAGS) -o treasure_manager treasure_manager.c

treasure_monitor: treasure_monitor.c
	$(CC) $(CFLAGS) -o treasure_monitor treasure_monitor.c

treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) -o treasure_hub treasure_hub.c

calculate_score: calculate_score.c
	$(CC) $(CFLAGS) -o calculate_score calculate_score.c

clean:
	rm -f treasure_manager treasure_monitor treasure_hub calculate_score *.o
