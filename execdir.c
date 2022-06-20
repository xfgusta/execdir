#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#define USAGE   "Usage: execdir [--help] [--version] [path] [command...]"
#define VERSION "0.1.0"

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

void usage_message() {
    fprintf(stderr, USAGE "\n");
    exit(1);
}

void help_message() {
    printf(USAGE "\n\n"
           "Options:\n"
           "  --help     display this help and exit\n"
           "  --version  output version information and exit\n\n"
           "Report bugs to <https://github.com/xfgusta/execdir/issues>\n");
    exit(0);
}

int main(int argc, char **argv) {
    int opt;
    int opt_index = 0;
    char *path;
    int shell_exec = 0;
    int help_opt, version_opt;

    help_opt = version_opt = 0;

    struct option long_opts[] = {
        {"help",    no_argument, &help_opt,    1},
        {"version", no_argument, &version_opt, 1},
        {0,         0,           0,            0}
    };

    while((opt = getopt_long(argc, argv, "", long_opts, &opt_index)) != -1) {
        switch(opt) {
            case '?':
                usage_message();
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if(help_opt) {
        help_message();
    } else if(version_opt) {
        printf("execdir version %s\n", VERSION);
        exit(0);
    }

    if(argc < 2) {
        usage_message();
    }

    path = *argv;
    argv += 1;

    if(chdir(path) == -1) {
        fprintf(stderr, "Cannot change directory: %s\n", strerror(errno));
        exit(1);
    }

    setenv("PWD", path, 1);

    exit(shell_exec ? sh_exec_cmd(argc, argv) : exec_cmd(argv));
}
