#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host> [count] [interval]\n", argv[0]);
        return 1;
    }

    int count = argc >= 3 ? atoi(argv[2]) : 10;
    int interval = argc >= 4 ? atoi(argv[3]) : 1;

    if (count <= 0 || interval <= 0) {
        fprintf(stderr, "Invalid count or interval\n");
        return 1;
    }

    char count_arg[16];
    char interval_arg[16];

    snprintf(count_arg, sizeof(count_arg), "%d", count);
    snprintf(interval_arg, sizeof(interval_arg), "%d", interval);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl(
            "/system/bin/ping",
            "ping",
            "-c",
            count_arg,
            "-i",
            interval_arg,
            argv[1],
            (char *)NULL
        );

        perror("execl");
        _exit(1);
    }

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}