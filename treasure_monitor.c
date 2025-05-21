#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

#define CONTROL_FILE "control.cmd"
#define STATUS_FILE "status.out"
#define MAX_LINE 256

volatile sig_atomic_t got_signal = 0;

void handle_usr1(int sig) { got_signal = 1; }

void write_status(const char *text) {
    int fd = open(STATUS_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, text, strlen(text));
        close(fd);
    }
}

void process_command() {
    int fd = open(CONTROL_FILE, O_RDONLY);
    if (fd < 0) return;

    char buf[MAX_LINE] = {0};
    read(fd, buf, sizeof(buf));
    close(fd);

    char cmd[32], arg1[64], arg2[64];
    sscanf(buf, "%s %s %s", cmd, arg1, arg2);

    if (strcmp(cmd, "list_hunts") == 0) {
        DIR *d = opendir(".");
        struct dirent *entry;
        char output[MAX_LINE * 20] = "";
        while ((entry = readdir(d))) {
            if (entry->d_type == DT_DIR && strncmp(entry->d_name, "hunt:", 5) == 0) {
                char path[128];
                snprintf(path, sizeof(path), "%s/treasures.bin", entry->d_name);
                int fd = open(path, O_RDONLY);
                if (fd >= 0) {
                    off_t size = lseek(fd, 0, SEEK_END);
                    close(fd);
                    char line[128];
                    snprintf(line, sizeof(line), "%s: %ld bytes\n", entry->d_name, size);
                    strcat(output, line);
                }
            }
        }
        closedir(d);
        write_status(output);
    }
    else if (strcmp(cmd, "list_treasures") == 0) {
        char path[128];
        snprintf(path, sizeof(path), "%s/treasures.bin", arg1);
        int fd = open(path, O_RDONLY);
        if (fd < 0) { write_status("Not found\n"); return; }

        struct Treasure {
            int id; char username[32]; float lat, lon; char clue[128]; int value;
        } t;
        char output[4096] = "";
        while (read(fd, &t, sizeof(t)) == sizeof(t)) {
            char line[256];
            snprintf(line, sizeof(line), "[%d] %s (%.2f, %.2f) - %s (%d)\n",
                     t.id, t.username, t.lat, t.lon, t.clue, t.value);
            strcat(output, line);
        }
        close(fd);
        write_status(output);
    }
    else if (strcmp(cmd, "view_treasure") == 0) {
        char path[128];
        snprintf(path, sizeof(path), "%s/treasures.bin", arg1);
        int fd = open(path, O_RDONLY);
        if (fd < 0) { write_status("Not found\n"); return; }

        struct Treasure {
            int id; char username[32]; float lat, lon; char clue[128]; int value;
        } t;
        int target_id = atoi(arg2);
        while (read(fd, &t, sizeof(t)) == sizeof(t)) {
            if (t.id == target_id) {
                char line[256];
                snprintf(line, sizeof(line), "[%d] %s (%.2f, %.2f) - %s (%d)\n",
                         t.id, t.username, t.lat, t.lon, t.clue, t.value);
                write_status(line);
                close(fd);
                return;
            }
        }
        close(fd);
        write_status("Not found\n");
    }
    else if (strcmp(cmd, "stop_monitor") == 0) {
        usleep(100000);
        write_status("Monitor stopped\n");
        exit(0);
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        if (got_signal) {
            got_signal = 0;
            process_command();
        }
        usleep(100000);
    }
    return 0;
}
