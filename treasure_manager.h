#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#define MAX_NAME 32
#define MAX_CLUE 128
#define RECORD_SIZE sizeof(struct Treasure)

struct Treasure {
    int id;
    char user[MAX_NAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
};

// Functii publice
void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int tid);
void remove_treasure(const char *hunt_id, int tid);
void remove_hunt(const char *hunt_id);

#endif
