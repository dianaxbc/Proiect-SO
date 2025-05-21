
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

struct Treasure {
    int id;
    char username[32];
    float lat, lon;
    char clue[128];
    int value;
};

struct ScoreNode {
    char user[32];
    int score;
    struct ScoreNode *next;
};

struct ScoreNode* find_or_create(struct ScoreNode **head, const char *user) {
    struct ScoreNode *cur = *head;
    while (cur) {
        if (strcmp(cur->user, user) == 0) return cur;
        cur = cur->next;
    }
    cur = malloc(sizeof(struct ScoreNode));
    strcpy(cur->user, user);
    cur->score = 0;
    cur->next = *head;
    *head = cur;
    return cur;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hunt_folder>\n", argv[0]);
        return 1;
    }

    char path[128];
    snprintf(path, sizeof(path), "%s/treasures.bin", argv[1]);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 1;

    struct Treasure t;
    struct ScoreNode *head = NULL;

    while (read(fd, &t, sizeof(t)) == sizeof(t)) {
        struct ScoreNode *node = find_or_create(&head, t.username);
        node->score += t.value;
    }
    close(fd);

    struct ScoreNode *cur = head;
    while (cur) {
        printf("%s: %d\n", cur->user, cur->score);
        cur = cur->next;
    }

    while (head) {
        struct ScoreNode *tmp = head;
        head = head->next;
        free(tmp);
    }

    return 0;
}
