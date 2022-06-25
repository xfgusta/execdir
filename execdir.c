#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <pwd.h>

#define USAGE "Usage: execdir [--help] [--version] [-s] [path] [command...]"
#define VERSION "0.1.0"
#define EXECDIR_FILE ".execdir"

// name:path record linked list
struct list {
    char *name;
    char *path;

    struct list *next;
};

// strdup wrapper
char *xstrdup(char *s) {
    char *str;

    str = strdup(s);
    if(!str) {
        fprintf(stderr, "Cannot allocate memory: %s\n", strerror(errno));
        exit(1);
    }

    return str;
}

// add a new name:path record on to the start of the list
struct list *list_prepend(struct list *list, char *name, char *path) {
    struct list *new_list;

    new_list = malloc(sizeof(struct list));
    if(!new_list) {
        fprintf(stderr, "Cannot allocate memory: %s\n", strerror(errno));
        exit(1);
    }

    new_list->name = xstrdup(name);
    new_list->path = xstrdup(path);
    new_list->next = list;

    return new_list;
}

// remove a name:path record from the list
struct list *list_remove(struct list *list, char *name) {
    struct list **prev_list = &list;
    struct list *current_list = NULL;

    while(*prev_list) {
        current_list = *prev_list;

        if(!strcmp(name, current_list->name)) {
            *prev_list = current_list->next;
            free(current_list->name);
            free(current_list->path);
            free(current_list);
            break;
        } else
            prev_list = &current_list->next;
    }

    return list;
}

// reverse the list
struct list *list_reverse(struct list *list) {
    struct list *prev_list = NULL;
    struct list *next_list;

    while(list) {
        next_list = list->next;
        list->next = prev_list;
        prev_list = list;
        list = next_list;
    }

    return prev_list;
}

// free all the memory used by the list
void list_free(struct list *list) {
    for(struct list *next_list; list; list = next_list) {
        next_list = list->next;
        free(list->name);
        free(list->path);
        free(list);
    }
}

// get the current user's home directory
char *get_home_dir() {
    char *home_dir;

    home_dir = getenv("HOME");

    // fall back to the passwd file
    if(!home_dir) {
        uid_t uid;
        struct passwd *pw;

        uid = getuid();
        pw = getpwuid(uid);

        if(!pw)
            return NULL;

        home_dir = pw->pw_dir;
    }

    return home_dir;
}

// return execdir file path
char *get_execdir_file_path() {
    char *path;
    char *home_dir;

    home_dir = get_home_dir();
    if(!home_dir) {
        fprintf(stderr, "Cannot get the home directory\n");
        exit(1);
    }

    path = malloc(strlen(home_dir) + strlen("/" EXECDIR_FILE) + 1);
    if(!path) {
        fprintf(stderr, "Cannot allocate memory: %s", strerror(errno));
        exit(1);
    }

    sprintf(path, "%s/" EXECDIR_FILE, home_dir);

    return path;
}

// parse execdir file and return a list of name:path records
struct list *get_list_from_file(char *filename) {
    struct list *list = NULL;
    FILE *file;
    char *line = NULL;
    size_t len;

    file = fopen(filename, "r");
    if(!file) {
        // return NULL when the file doesn't exist
        if(errno == ENOENT)
            return NULL;

        fprintf(stderr, "Cannot open \"%s\" file: %s\n", filename,
                strerror(errno));
        exit(1);
    }

    while(getline(&line, &len, file) != -1) {
        char *name = line;
        char *path;
        int before_newline;

        path = strchr(line, ':');

        // delimiter missing
        if(!path)
            continue;

        // end the name string before the delimiter and skip the delimiter
        *path++ = '\0';

        // end the path string before the trailing newline
        before_newline = strcspn(path, "\n");
        path[before_newline] = '\0';

        // name or path missing
        if(!(*name && *path))
            continue;

        list = list_prepend(list, name, path);
    }

    free(line);
    fclose(file);

    // reverse the list to match the order in the file
    list = list_reverse(list);

    return list;
}

// getcwd wrapper
char *xgetcwd() {
    char *cwd;
    long size;
    char *buf;

    size = pathconf(".", _PC_PATH_MAX);
    if(size == -1)
        return NULL;

    buf = malloc(size);
    if(!buf)
        return NULL;

    cwd = getcwd(buf, size);

    return cwd;
}

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
    char *cwd;
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

    cwd = xgetcwd();
    if(!cwd) {
        fprintf(stderr, "Cannot get the current working directory\n");
        exit(1);
    }

    if(chdir(path) == -1) {
        fprintf(stderr, "Cannot change directory: %s\n", strerror(errno));
        exit(1);
    }

    // set old and new path to reflect the directory change
    setenv("OLDPWD", cwd, 1);
    setenv("PWD", path, 1);

    free(cwd);

    exit(sh_exec_opt ? sh_exec_cmd(argc, argv) : exec_cmd(argv));
}
