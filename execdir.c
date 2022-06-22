#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#define USAGE   "Usage: execdir [--help] [--version] [-s] [path] [command...]"
#define VERSION "0.1.0"

// join all the arguments by adding a space between them
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

        // don't add a space at the end
        if(i + 1 != argc)
            strcat(str, " ");
    }

    return str;
}

int sh_exec_cmd(int argc, char **argv) {
    int status;
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
    int status;

    status = execvp(*argv, argv);
    if(status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        exit(1);
    }

    return status;
}

void usage_message() {
    fprintf(stderr, USAGE "\n");
    exit(1);
}

void help_message() {
    printf(USAGE "\n\n"
           "Options:\n"
           "  --help       display this help and exit\n"
           "  --version    output version information and exit\n"
           "  -s, --shell  execute the command as a shell command\n\n"
           "Report bugs to <https://github.com/xfgusta/execdir/issues>\n");
    exit(0);
}

int main(int argc, char **argv) {
    int opt;
    int opt_index = 0;
    char *path;
    int help_opt = 0;
    int version_opt = 0;
    int sh_exec_opt = 0;

    struct option long_opts[] = {
        {"help",    no_argument, &help_opt,    1},
        {"version", no_argument, &version_opt, 1},
        {"shell",   no_argument, &sh_exec_opt, 1},
        {0,         0,           0,            0}
    };

    while((opt = getopt_long(argc, argv, "s", long_opts, &opt_index)) != -1) {
        switch(opt) {
            case 's':
                sh_exec_opt = 1;
                break;
            case '?':
                usage_message();
                break;
        }
    }

    // skip to the non-option arguments
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

    // save and skip the path argument
    path = *argv;
    argc -= 1;
    argv += 1;

    if(chdir(path) == -1) {
        fprintf(stderr, "Cannot change directory: %s\n", strerror(errno));
        exit(1);
    }

    // set the new path to reflect the directory change
    setenv("PWD", path, 1);

    exit(sh_exec_opt ? sh_exec_cmd(argc, argv) : exec_cmd(argv));
}
