#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#define LOG_FILE "log.txt"
#define TREASURE_FILE "treasures.txt"

void log_op(const char *hunt_id, const char *operation);
void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int id);
void remove_treasure(const char *hunt_id, int id);
void remove_hunt(const char *hunt_id);

#endif
