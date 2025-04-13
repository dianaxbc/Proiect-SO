#include "treasure_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"


static void ensure_dir(const char *dir) {
    mkdir(dir, 0755); //creeaza dir ul daca nu exista
}

// logarea operatiunii
static void log_op(const char *hunt_id, const char *operation) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        dprintf(fd, "[%ld] %s\n", time(NULL), operation);
        close(fd);

        char linkname[256];
        snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
        symlink(log_path, linkname); // se ignora daca deja exista
    }
}

void add_treasure(const char *hunt_id) {
    ensure_dir(hunt_id);

    struct Treasure t;
    printf("User: ");
    scanf("%s", t.user);
    printf("Latitude: ");
    scanf("%f", &t.latitude);
    printf("Longitude: ");
    scanf("%f", &t.longitude);
    getchar(); 
    printf("Clue: ");
    fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Value: ");
    scanf("%d", &t.value);

    char path[128];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    t.id = (int)(lseek(fd, 0, SEEK_END) / RECORD_SIZE);
    write(fd, &t, RECORD_SIZE);
    close(fd);

    log_op(hunt_id, "Added treasure");
}

void list_treasures(const char *hunt_id) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat st;
    if (stat(path, &st) != 0) {
        perror("Cannot stat file");
        return;
    }

    printf("Hunt: %s\nSize: %ld bytes\nLast modified: %ld\n", hunt_id, st.st_size, st.st_mtime);

    int fd = open(path, O_RDONLY);
    struct Treasure t;
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        printf("ID %d: %s (%.2f, %.2f) [%s] - %d gold\n", t.id, t.user, t.latitude, t.longitude, t.clue, t.value);
    }
    close(fd);

    log_op(hunt_id, "Listed treasures");
}

void view_treasure(const char *hunt_id, int tid) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(path, O_RDONLY);
    lseek(fd, tid * RECORD_SIZE, SEEK_SET);
    struct Treasure t;

    if (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        printf("Treasure #%d\nUser: %s\nGPS: %.2f, %.2f\nClue: %s\nValue: %d\n", t.id, t.user, t.latitude, t.longitude, t.clue, t.value);
    } else {
        printf("Not found\n");
    }
    close(fd);

    log_op(hunt_id, "Viewed treasure");
}

void remove_treasure(const char *hunt_id, int tid) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct Treasure t;
    int found = 0;
    int total = 0;
    struct Treasure *buffer = NULL;

    // citeste toate comorile si le pastreaza mai putin pe cea stearsa 
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        if (t.id != tid) {
            // reindexez si adaug in buffer
            total++;
            buffer = realloc(buffer, total * RECORD_SIZE);
            buffer[total - 1] = t;  // copiez structura in buffer 
        } else {
            found = 1;
        }
    }

    if (!found) {
        printf("Treasure with ID %d not found.\n", tid);
        free(buffer);
        close(fd);
        return;
    }

    // trunchiem fisierul pt a l
    ftruncate(fd, 0);

    // rescriu fisierul fara comoara stearsa
    for (int i = 0; i < total; i++) {
        if (write(fd, &buffer[i], RECORD_SIZE) != RECORD_SIZE) {
            perror("write");
            free(buffer);
            close(fd);
            return;
        }
    }

    free(buffer);
    close(fd);

    printf("Treasure %d removed.\n", tid);
}


void remove_hunt(const char *hunt_id) {
    char path[128];

    // verific daca exista directorul
    snprintf(path, sizeof(path), "%s", hunt_id);
    struct stat statbuf;
    if (stat(path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        printf("Hunt '%s' does not exist or is not a directory.\n", hunt_id);
        return;
    }

    // sterge fisier comori 
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    if (unlink(path) == 0) {
        printf("Treasure file removed.\n");
    } else {
        perror("unlink treasure file");
    }

    // sterge fisier de logare
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    if (unlink(path) == 0) {
        printf("Log file removed.\n");
    } else {
        perror("unlink log file");
    }

    //elimina leg simbolica
    snprintf(path, sizeof(path), "logged_hunt-%s", hunt_id);
    if (unlink(path) == 0) {
        printf("Symlink removed.\n");
    } else {
        perror("unlink symlink");
    }

    // sterge hunt directory
    if (rmdir(hunt_id) == 0) {
        printf("Hunt '%s' removed successfully.\n", hunt_id);
    } else {
        perror("rmdir");
        printf("Could not remove hunt '%s'. Please check the directory.\n", hunt_id);
    }
}