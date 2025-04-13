#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_manager.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: treasure_manager <command> <hunt_id> [<id>]\n");
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_treasures(argv[2]);
    } else if (strcmp(argv[1], "view") == 0 && argc == 4) {
        view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "remove_treasure") == 0 && argc == 4) {
        remove_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "remove_hunt") == 0) {
        remove_hunt(argv[2]);
    } else {
        printf("Unknown command or wrong arguments.\n");
    }

    return 0;
}
