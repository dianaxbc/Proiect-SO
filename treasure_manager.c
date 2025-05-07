#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h> // doar pentru snprintf si atoi
#include "treasure_manager.h"

void log_op(const char *hunt_id, const char *operation) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        write(fd, operation, strlen(operation));
        write(fd, "\n", 1);
        close(fd);
    }
}

void add_treasure(const char *hunt_id) {
    mkdir(hunt_id, 0755);

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    const char *content = "Treasure content\n";
    write(fd, content, strlen(content));
    close(fd);

    log_op(hunt_id, "add");
}

void list_treasures(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(1, buf, n);
    }

    close(fd);
    log_op(hunt_id, "list");
}

void view_treasure(const char *hunt_id, int id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    char buf[256];
    int current = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (current == id) {
            write(1, buf, n);
            break;
        }
        current++;
    }

    close(fd);
    log_op(hunt_id, "view");
}

void remove_treasure(const char *hunt_id, int id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/temp.txt", hunt_id);
    int tmp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tmp_fd < 0) {
        close(fd);
        return;
    }

    char buf[256];
    int current = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (current != id) {
            write(tmp_fd, buf, n);
        }
        current++;
    }

    close(fd);
    close(tmp_fd);
    rename(temp_path, path);
    log_op(hunt_id, "remove_treasure");
}

void remove_hunt(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    unlink(path);
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    unlink(path);
    rmdir(hunt_id);
    log_op(hunt_id, "remove_hunt");
}
