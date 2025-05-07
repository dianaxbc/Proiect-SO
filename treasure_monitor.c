#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>

#define CONTROL_FILE "control"
#define STATUS_FILE "status"
#define PID_FILE "monitor.pid"
#define MAX_CMD 128

char hunt_id[128];

void handle_sigusr1(int sig) {
    char ctrl_path[256];
    snprintf(ctrl_path, sizeof(ctrl_path), "%s/%s", hunt_id, CONTROL_FILE);
    int fd = open(ctrl_path, O_RDONLY);
    if (fd < 0) return;

    char cmd[MAX_CMD] = {0};
    read(fd, cmd, sizeof(cmd) - 1);
    close(fd);

    char status_path[256];
    snprintf(status_path, sizeof(status_path), "%s/%s", hunt_id, STATUS_FILE);
    int out = open(status_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out >= 0) {
        dprintf(out, "Executed command: %s\n", cmd);
        close(out);
    }

    // Send SIGUSR2 back to hub
    kill(getppid(), SIGUSR2);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(1, "Usage: treasure_monitor <hunt_id>\n", 35);
        return 1;
    }

    strncpy(hunt_id, argv[1], sizeof(hunt_id) - 1);
    signal(SIGUSR1, handle_sigusr1);

    char pid_path[256];
    snprintf(pid_path, sizeof(pid_path), "%s/%s", hunt_id, PID_FILE);
    int fd = open(pid_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        char pid_str[20];
        int len = snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
        write(fd, pid_str, len);
        close(fd);
    }

    while (1) {
        pause(); // Wait for signals
    }

    return 0;
}
