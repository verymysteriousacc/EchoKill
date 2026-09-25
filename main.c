#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host> [count]\n", argv[0]);
        return 1;
    }

    int count = argc >= 3 ? atoi(argv[2]) : 4;

    if (count <= 0) {
        fprintf(stderr, "Invalid count\n");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        char count_arg[16];
        snprintf(count_arg, sizeof(count_arg), "%d", count);

        execl(
            "/system/bin/ping",
            "ping",
            "-c",
            count_arg,
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