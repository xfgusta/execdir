#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define USAGE "Usage: execdir [-h] [-v] [-s] [-a NAME PATH] [-r NAME] [-l] " \
              "[ARGS...]"

#define VERSION "0.3.0"

#define EXECDIR_FILE ".execdir"

#define print_error(...) fprintf(stderr, "execdir: " __VA_ARGS__);

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
        print_error("cannot allocate memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return str;
}

// add a new name:path record on to the start of the list
struct list *list_prepend(struct list *list, char *name, char *path) {
    struct list *new_list;

    new_list = malloc(sizeof(struct list));
    if(!new_list) {
        print_error("cannot allocate memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    new_list->name = xstrdup(name);
    new_list->path = xstrdup(path);
    new_list->next = list;

    return new_list;
}

// return a path found by name in the list
char *list_get_path_by_name(struct list *list, char *name) {
    for(struct list *dir = list; dir; dir = dir->next) {
        if(!strcmp(name, dir->name))
            return dir->path;
    }

    return NULL;
}

// remove a name:path record from the list
struct list *list_remove(struct list *list, char *name, int *found) {
    struct list **prev_list = &list;
    struct list *current_list = NULL;
    *found = 0;

    while(*prev_list) {
        current_list = *prev_list;

        if(!strcmp(name, current_list->name)) {
            *prev_list = current_list->next;
            free(current_list->name);
            free(current_list->path);
            free(current_list);
            *found = 1;
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
        print_error("cannot get the home directory\n");
        exit(EXIT_FAILURE);
    }

    path = malloc(strlen(home_dir) + strlen("/" EXECDIR_FILE) + 1);
    if(!path) {
        print_error("cannot allocate memory: %s", strerror(errno));
        exit(EXIT_FAILURE);
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

        print_error("cannot open \"%s\" file: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
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

// save a list to the execdir file
void save_list_to_file(char *filename, struct list *list) {
    FILE *file;

    file = fopen(filename, "w");
    if(!file) {
        print_error("cannot open \"%s\" file: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for(struct list *dir = list; dir; dir = dir->next) {
        fprintf(file, "%s:%s\n", dir->name, dir->path);
    }

    fclose(file);
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
        print_error("cannot allocate memory for command string\n");
        exit(EXIT_FAILURE);
    }

    status = system(cmd);
    if(status == -1) {
        print_error("failed to execute command: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    free(cmd);

    return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;
}

int exec_cmd(char **argv) {
    int status;

    status = execvp(*argv, argv);
    if(status == -1) {
        print_error("failed to execute command: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return status;
}

void usage_message() {
    fprintf(stderr, USAGE "\n");
    exit(EXIT_FAILURE);
}

void help_message() {
    printf(USAGE "\n\n"
           "Options:\n"
           "  -h            display this help and exit\n"
           "  -v            output version information and exit\n"
           "  -s            execute the command as a shell command\n"
           "  -a NAME PATH  add an alias for a path\n"
           "  -r NAME       remove an alias\n"
           "  -l            list all aliases\n\n"
           "Report bugs to <https://github.com/xfgusta/execdir/issues>\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    int opt;
    char *execdir_file_path = 0;
    struct list *list = NULL;
    char *cwd;
    char *path;
    struct stat st;
    int help_opt = 0;
    int version_opt = 0;
    int sh_exec_opt = 0;
    int add_alias_opt = 0;
    int rm_alias_opt = 0;
    int ls_alias_opt = 0;

    while((opt = getopt(argc, argv, "hvsarl")) != -1) {
        switch(opt) {
            case 'h':
                help_opt = 1;
                break;
            case 'v':
                version_opt = 1;
                break;
            case 's':
                sh_exec_opt = 1;
                break;
            case 'a':
                add_alias_opt = 1;
                break;
            case 'r':
                rm_alias_opt = 1;
                break;
            case 'l':
                ls_alias_opt = 1;
                break;
            case '?':
                exit(EXIT_FAILURE);
        }
    }

    // skip to the non-option arguments
    argc -= optind;
    argv += optind;

    if(help_opt) {
        help_message();
    } else if(version_opt) {
        printf("execdir version %s\n", VERSION);
        exit(EXIT_SUCCESS);
    }

    // load aliases
    execdir_file_path = get_execdir_file_path();
    list = get_list_from_file(execdir_file_path);

    if(add_alias_opt) {
        if(argc != 2) {
            print_error("-a requires two arguments\n");
            exit(EXIT_FAILURE);
        }

        list = list_prepend(list, argv[0], argv[1]);
        list = list_reverse(list);

        save_list_to_file(execdir_file_path, list);
    } else if(rm_alias_opt) {
        int found;

        if(argc != 1) {
            print_error("-r requires one argument\n");
            exit(EXIT_FAILURE);
        }

        list = list_remove(list, argv[0], &found);
        if(!found) {
            print_error("\"%s\" alias not found\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        save_list_to_file(execdir_file_path, list);
    } else if(ls_alias_opt) {
        size_t max_name_len = 0;

        // get the longest alias name length
        for(struct list *dir = list; dir; dir = dir->next) {
            size_t len = strlen(dir->name);
            if(len > max_name_len)
                max_name_len = len;
        }

        for(struct list *dir = list; dir; dir = dir->next) {
            int count = 4 + max_name_len - strlen(dir->name);
            printf("%s%*s%s\n", dir->name, count, "", dir->path);
        }
    }

    if(add_alias_opt || rm_alias_opt || ls_alias_opt) {
        free(execdir_file_path);
        list_free(list);
        exit(EXIT_SUCCESS);
    }

    if(argc < 2) {
        usage_message();
    }

    // save and skip the path argument
    path = *argv;
    argc -= 1;
    argv += 1;

    // try to get an alias if path doesn't exist
    if(stat(path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char *name = path;

        path = list_get_path_by_name(list, name);
        if(!path) {
            print_error("path or alias for path \"%s\" not found\n", name);
            exit(EXIT_FAILURE);
        }
    }

    cwd = xgetcwd();
    if(!cwd) {
        print_error("cannot get the current working directory\n");
        exit(EXIT_FAILURE);
    }

    if(chdir(path) == -1) {
        print_error("cannot change directory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set old and new path to reflect the directory change
    setenv("OLDPWD", cwd, 1);
    setenv("PWD", path, 1);

    free(cwd);
    free(execdir_file_path);
    list_free(list);

    exit(sh_exec_opt ? sh_exec_cmd(argc, argv) : exec_cmd(argv));
}
