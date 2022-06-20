#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

char *argv_to_str(int argc, char **argv) {
    char *str;
    size_t str_len = 0;

    for(int i = 0; i < argc; i++) {
        str_len += strlen(argv[i]);

        if(i + 1 != argc)
            str_len++;
    }

    str = calloc(1, str_len + 1);
    if(!str)
        return NULL;

    for(int i = 0; i < argc; i++) {
        strcat(str, argv[i]);

        if(i + 1 != argc)
            strcat(str, " ");
    }

    return str;
}

int sh_exec_cmd(int argc, char **argv) {
    int status = 0;
    char *cmd;

    cmd = argv_to_str(argc, argv);
    if(!cmd) {
        fprintf(stderr, "Cannot allocate memory for command string\n");
        exit(1);
    }

    status = system(cmd);
    if(status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        exit(1);
    }

    free(cmd);

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int exec_cmd(char **argv) {
    return execvp(*argv, argv);
}

int main(int argc, char **argv) {
    char *path;
    int shell_exec = 0;

    if(argc < 2) {
        fprintf(stderr, "Usage: execdir [path] [command...]\n");
        exit(1);
    }

    path = argv[1];

    if(chdir(path) == -1) {
        fprintf(stderr, "Cannot change directory: %s\n", strerror(errno));
        exit(1);
    }

    setenv("PWD", path, 1);

    argc -= 2;
    argv += 2;

    exit(shell_exec ? sh_exec_cmd(argc, argv) : exec_cmd(argv));
}
