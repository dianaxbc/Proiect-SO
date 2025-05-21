#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define MAX_USERNAME 32
#define MAX_CLUE 128
#define LOG_FILE "logged_hunt"
#define TREASURE_FILE "treasures.bin"

struct Treasure {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
};

void log_action(const char *hunt_id, const char *msg) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        write(fd, msg, strlen(msg));
        write(fd, "\n", 1);
        close(fd);
    }
    char symlink_name[64];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    if (access(symlink_name, F_OK) == -1) {
        symlink(log_path, symlink_name);
    }
}

void add_treasure(const char *hunt_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    mkdir(dir_path, 0755);

    struct Treasure t;
    t.id = (int)time(NULL) % 10000;
    write(1, "Username: ", 10); read(0, t.username, MAX_USERNAME);
    write(1, "Latitude: ", 10); scanf("%f", &t.latitude);
    write(1, "Longitude: ", 11); scanf("%f", &t.longitude);
    write(1, "Clue: ", 6); read(0, t.clue, MAX_CLUE);
    write(1, "Value: ", 7); scanf("%d", &t.value);

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", dir_path, TREASURE_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write(fd, &t, sizeof(t));
    close(fd);

    log_action(hunt_id, "Treasure added");
}

void list_treasures(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) { write(1, "Hunt not found\n", 16); return; }

    struct stat st;
    fstat(fd, &st);
    dprintf(1, "Hunt: %s\nSize: %ld bytes\n", hunt_id, st.st_size);

    struct Treasure t;
    while (read(fd, &t, sizeof(t)) == sizeof(t)) {
        dprintf(1, "[%d] %s (%.2f, %.2f) - %s (%d)\n",
                t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }
    close(fd);
    log_action(hunt_id, "Listed treasures");
}

void remove_hunt(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    unlink(path);
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    unlink(path);
    rmdir(hunt_id);

    char symlink_name[64];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        write(1, "Usage: ./treasure_manager <command> <hunt_id>\n", 46);
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) add_treasure(argv[2]);
    else if (strcmp(argv[1], "list") == 0) list_treasures(argv[2]);
    else if (strcmp(argv[1], "remove_hunt") == 0) remove_hunt(argv[2]);
    else write(1, "Invalid command\n", 17);

    return 0;
}
