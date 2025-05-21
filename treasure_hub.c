
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CONTROL_FILE "control.cmd"
#define STATUS_FILE "status.out"

pid_t monitor_pid = -1;
int monitor_ready = 0;

void send_command(const char *cmd) {
    int fd = open(CONTROL_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, cmd, strlen(cmd));
    close(fd);
    kill(monitor_pid, SIGUSR1);
}

void read_status() {
    usleep(200000);
    int fd = open(STATUS_FILE, O_RDONLY);
    if (fd >= 0) {
        char buf[4096];
        int n = read(fd, buf, sizeof(buf) - 1);
        buf[n] = '\0';
        printf("%s", buf);
        close(fd);
    }
}

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    monitor_ready = 0;
    printf("Monitor exited.\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    char input[128];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "start_monitor") == 0 && monitor_ready == 0) {
            monitor_pid = fork();
            if (monitor_pid == 0) execl("./treasure_monitor", "treasure_monitor", NULL);
            else monitor_ready = 1;
        }
        else if (strncmp(input, "list_hunts", 10) == 0 && monitor_ready) {
            send_command("list_hunts"); read_status();
        }
        else if (strncmp(input, "list_treasures", 14) == 0 && monitor_ready) {
            char *hunt = input + 15;
            char buf[128]; snprintf(buf, sizeof(buf), "list_treasures %s", hunt);
            send_command(buf); read_status();
        }
        else if (strncmp(input, "view_treasure", 13) == 0 && monitor_ready) {
            char *rest = input + 14;
            char buf[128]; snprintf(buf, sizeof(buf), "view_treasure %s", rest);
            send_command(buf); read_status();
        }
        else if (strcmp(input, "stop_monitor") == 0 && monitor_ready) {
            send_command("stop_monitor");
        }
        else if (strcmp(input, "exit") == 0) {
            if (monitor_ready) printf("Monitor still running. Use stop_monitor first.\n");
            else break;
        }
        else {
            printf("Unknown or invalid command.\n");
        }
    }
    return 0;
}
