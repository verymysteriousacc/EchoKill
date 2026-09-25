#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    printf("Hello welcome the name sucks (10 packets per sec)\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host> [count] [interval]\n", argv[0]);
        return 1;
    }

    int count = argc >= 3 ? atoi(argv[2]) : 10;
    double interval = argc >= 4 ? atof(argv[3]) : 0.5;

    if (count <= 0) {
        fprintf(stderr, "Invalid count\n");
        return 1;
    }

    useconds_t delay = (useconds_t)(interval * 1000000.0);

    for (int i = 0; i < count; i++) {
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
                "10",
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

        if (i + 1 < count)
            usleep(delay);
    }

    return 0;
}