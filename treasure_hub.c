#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>   
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_LINE 256
#define CONTROL_FILE "control"
#define STATUS_FILE "status"
#define PID_FILE "monitor.pid"

volatile sig_atomic_t got_response = 0;

void handle_sigusr2(int sig) {
    got_response = 1;
}

// citeste o linie de la tastatura (stdin) folosind read
ssize_t read_line(int fd, char *buffer, size_t max_len) {
    size_t i = 0;
    char c;
    while (i < max_len - 1) {
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) break;
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}

// scrie comanda in fisierul control
void send_command(const char *hunt_id, const char *command) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, CONTROL_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        write(2, "Eroare la open control file\n", 28);
        return;
    }
    write(fd, command, strlen(command));
    close(fd);
}

// citeste pid-ul monitorului din fisier
pid_t get_monitor_pid(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, PID_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        write(2, "Eroare la open pid file\n", 25);
        return -1;
    }
    char buf[20];
    int r = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (r <= 0) return -1;
    buf[r] = '\0';
    return (pid_t)atoi(buf);
}

// citeste si afiseaza statusul returnat de monitor
void read_status(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, STATUS_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        write(2, "Eroare la open status file\n", 27);
        return;
    }

    char buf[512];
    ssize_t bytes;
    while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
        write(1, buf, bytes);
    }

    close(fd);
}

int main() {
    signal(SIGUSR2, handle_sigusr2);

    char line[MAX_LINE];
    write(1, "Treasure Hub - enter commands ('exit' to quit):\n", 49);

    while (1) {
        write(1, "> ", 2);

        ssize_t len = read_line(0, line, sizeof(line));
        if (len <= 0) break;

        if (strcmp(line, "exit") == 0) break;

        // Separam comanda si hunt_id-ul
        char *cmd = strtok(line, " ");
        char *hunt_id = strtok(NULL, " ");

        if (!cmd || !hunt_id) {
            write(1, "Comanda invalida. Format: <cmd> <hunt_id>\n", 42);
            continue;
        }

        pid_t mon_pid = get_monitor_pid(hunt_id);
        if (mon_pid < 0) {
            write(1, "PID-ul monitorului nu a fost gasit\n", 35);
            continue;
        }

        send_command(hunt_id, cmd);
        kill(mon_pid, SIGUSR1);

        // asteapta semnalul SIGUSR2 de la monitor
        got_response = 0;
        int tries = 50;
        while (!got_response && tries-- > 0) {
            usleep(100000); // 100ms
        }

        if (got_response) {
            read_status(hunt_id);
        } else {
            write(1, "Monitorul nu a raspuns.\n", 25);
        }
    }

    return 0;
}
