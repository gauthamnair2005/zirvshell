#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <zirv/syscall.h>

#define MAX_CMD_LEN 256
#define MAX_ARGS    16

static void print_prompt(void)
{
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) > 0) {
        printf("%s \xE2\x82\xB9 ", cwd);
    } else {
        printf("\xE2\x82\xB9 ");
    }
}

static int read_line(char *buf, int maxlen)
{
    int pos = 0;
    while (pos < maxlen - 1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) return -1;
        if (c == '\r' || c == '\n') {
            buf[pos] = '\0';
            write(STDOUT_FILENO, "\n", 1);
            return pos;
        }
        if ((c == '\b' || c == 127) && pos > 0) {
            pos--;
            write(STDOUT_FILENO, "\b \b", 3);
            continue;
        }
        if (c >= ' ') {
            buf[pos++] = c;
            write(STDOUT_FILENO, &c, 1);
        }
    }
    buf[pos] = '\0';
    return pos;
}

static int parse_args(char *cmd, char *argv[], int max_args)
{
    int argc = 0;
    char *p = cmd;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        if (argc >= max_args - 1) break;
        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p) { *p = '\0'; p++; }
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) { *p = '\0'; p++; }
        }
    }
    argv[argc] = NULL;
    return argc;
}

static void print_help_header(void)
{
    printf("ZirvShell MOSIX Commands:\n");
    printf("  HELP (General)\n");
    printf("    help / help me please           Show this help\n");
    printf("    clear / clear the screen        Clear the screen\n");
    printf("    exit                             Exit the shell\n");
    printf("\n");
    printf("  FILE OPERATIONS (English / POSIX)\n");
    printf("    create file <path>              Create an empty file\n");
    printf("    create folder / mkdir <path>    Create a directory\n");
    printf("    delete file / rm / unlink <p>   Delete a file\n");
    printf("    delete folder / rmdir <path>    Delete an empty directory\n");
    printf("    rename / mv <old> <new>         Rename a file or directory\n");
    printf("    copy / cp <src> <dst>           Copy a file\n");
    printf("    cat / display <file>            Display file contents\n");
    printf("    ls / list                       List directory contents\n");
    printf("    show files                      List directory (long form)\n");
    printf("\n");
    printf("  NAVIGATION\n");
    printf("    go to / cd <path>               Change directory\n");
    printf("    pwd / where                     Print working directory\n");
    printf("\n");
    printf("  SYSTEM INFO\n");
    printf("    date                             Print system date/time\n");
    printf("    settime <date> <time>            Set date/time (YYYY-MM-DD HH:MM:SS)\n");
    printf("    uptime                           Print system uptime\n");
    printf("    timezone [UTC+/-H:MM]            Show or set timezone\n");
    printf("    uname                            Print system information\n");
    printf("    whoami                           Show current user\n");
    printf("    show system status               Show system info\n");
    printf("\n");
    printf("  NETWORKING\n");
    printf("    ping [<ip>]                      Ping a host (default: 10.0.2.2)\n");
    printf("    ifconfig                         Show network interfaces\n");
    printf("\n");
    printf("  EXECUTION\n");
    printf("    echo / say <text>                Print text\n");
    printf("    run / exec <binary>              Execute a program\n");
    printf("\n");
    printf("  POWER\n");
    printf("    reboot                           Reboot the system\n");
    printf("    shutdown / poweroff              Shut down the system\n");
    printf("    suspend                          Suspend the system\n");
}

static int cmd_help(void)
{
    print_help_header();
    return 0;
}

static int cmd_clear(void)
{
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
    return 0;
}

static int cmd_whoami(void)
{
    printf("root\n");
    return 0;
}

static int cmd_echo(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\n");
    return 0;
}

static int cmd_pwd(void)
{
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) > 0) {
        printf("%s\n", cwd);
    }
    return 0;
}

static int cmd_uname(void)
{
    printf("Zirvium\n");
    return 0;
}

static int cmd_ls(int argc, char *argv[])
{
    const char *dir = (argc > 1) ? argv[1] : NULL;
    char buf[256];
    if (!dir) {
        if (getcwd(buf, sizeof(buf)) <= 0) return -1;
        dir = buf;
    }

    int fd = open(dir, 0);
    if (fd < 0) {
        printf("ls: cannot open '%s'\n", dir);
        return -1;
    }

    struct dirent ents[32];
    int n = getdents(fd, ents, 32);
    close(fd);

    if (n < 0) {
        printf("ls: error reading directory\n");
        return -1;
    }

    for (int i = 0; i < n; i++) {
        printf("  %s\n", ents[i].d_name);
    }
    return 0;
}

static int cmd_cat(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: cat <file>\n");
        return -1;
    }
    printf("cat: %s: not yet implemented\n", argv[1]);
    return 0;
}

static const char *day_name(int d)
{
    static const char *names[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    if (d < 0 || d > 6) return "???";
    return names[d];
}

static int cmd_date(void)
{
    struct datetime dt;
    if (getdatetime(&dt) < 0) {
        printf("date: unable to read system time\n");
        return -1;
    }
    int tz = gettz();
    char tz_sign = (tz >= 0) ? '+' : '-';
    int tz_abs = (tz >= 0) ? tz : -tz;
    printf("%s %04d-%02d-%02d %02d:%02d:%02d UTC%c%02d:%02d\n",
           day_name(0),
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second,
           tz_sign, tz_abs / 60, tz_abs % 60);
    return 0;
}

static int parse_int(const char **p)
{
    int val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10 + (**p - '0');
        (*p)++;
    }
    return val;
}

static int cmd_settime(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: settime YYYY-MM-DD HH:MM:SS\n");
        return -1;
    }
    struct datetime dt;
    const char *p = argv[1];
    dt.year = parse_int(&p);
    if (*p != '-') { printf("settime: invalid date format (use YYYY-MM-DD)\n"); return -1; }
    p++;
    dt.month = parse_int(&p);
    if (*p != '-') { printf("settime: invalid date format (use YYYY-MM-DD)\n"); return -1; }
    p++;
    dt.day = parse_int(&p);
    if (*p != '\0') { printf("settime: invalid date format (use YYYY-MM-DD)\n"); return -1; }

    p = argv[2];
    dt.hour = parse_int(&p);
    if (*p != ':') { printf("settime: invalid time format (use HH:MM:SS)\n"); return -1; }
    p++;
    dt.minute = parse_int(&p);
    if (*p != ':') { printf("settime: invalid time format (use HH:MM:SS)\n"); return -1; }
    p++;
    dt.second = parse_int(&p);
    if (*p != '\0') { printf("settime: invalid time format (use HH:MM:SS)\n"); return -1; }

    if (setdatetime(&dt) < 0) {
        printf("settime: failed to set system time\n");
        return -1;
    }
    printf("System time set.\n");
    return 0;
}

static int cmd_uptime(void)
{
    uint64_t secs = uptime();
    unsigned long days  = (unsigned long)(secs / 86400); secs %= 86400;
    unsigned long hours = (unsigned long)(secs / 3600);  secs %= 3600;
    unsigned long mins  = (unsigned long)(secs / 60);    secs %= 60;
    if (days > 0)
        printf("%lu days, %02lu:%02lu:%02lu\n",
               days, hours, mins, (unsigned long)secs);
    else
        printf("%02lu:%02lu:%02lu\n",
               hours, mins, (unsigned long)secs);
    return 0;
}

static int cmd_timezone(int argc, char *argv[])
{
    if (argc < 2) {
        int tz = gettz();
        char sign = (tz >= 0) ? '+' : '-';
        int abs = (tz >= 0) ? tz : -tz;
        printf("UTC%c%02d:%02d\n", sign, abs / 60, abs % 60);
        return 0;
    }
    const char *raw = argv[1];
    const char *p = raw;
    int sign = 1;
    if (*p == '+') { sign = 1; p++; }
    else if (*p == '-') { sign = -1; p++; }
    int hours = 0;
    while (*p >= '0' && *p <= '9') { hours = hours * 10 + (*p - '0'); p++; }
    int mins = 0;
    if (*p == ':') {
        p++;
        while (*p >= '0' && *p <= '9') { mins = mins * 10 + (*p - '0'); p++; }
    }
    int tz = (hours * 60 + mins) * sign;
    if (settz(tz) < 0) {
        printf("timezone: failed to set\n");
        return -1;
    }
    printf("Parsed '%s' -> %s%02d:%02d (%d min). Timezone set.\n",
           raw, sign < 0 ? "-" : "+", hours, mins, tz);
    return 0;
}

static int cmd_reboot(void)
{
    printf("Rebooting...\n");
    reboot();
    return 0;
}

static int cmd_shutdown(void)
{
    printf("Shutting down...\n");
    shutdown();
    return 0;
}

static int cmd_suspend(void)
{
    printf("Suspend: not supported on this hardware\n");
    return 1;
}

/* ── MOSIX English: show ─────────────────────────────────────────────────── */
static int cmd_show(int argc, char *argv[])
{
    if (argc < 2) {
        printf("show what? Try: show files, show system status\n");
        return -1;
    }
    if (strcmp(argv[1], "files") == 0) {
        return cmd_ls(0, NULL);
    } else if (argc >= 3 && strcmp(argv[1], "system") == 0
                         && strcmp(argv[2], "status") == 0) {
        int pid = getpid();
        printf("Zirvium System Status:\n");
        printf("  PID:        %d\n", pid);
        printf("  Kernel:     Zirvium\n");
        printf("  Userspace:  ZirvShell\n");
    } else {
        printf("Unknown show command.\n");
    }
    return 0;
}

/* ── MOSIX English: go to / cd ──────────────────────────────────────────── */
static int cmd_go(int argc, char *argv[])
{
    const char *path = NULL;
    if (argc >= 3 && strcmp(argv[1], "to") == 0) {
        path = argv[2];
    } else if (argc >= 2) {
        path = argv[1];
    }
    if (!path) {
        printf("Usage: go to <path>  or  cd <path>\n");
        return -1;
    }
    if (chdir(path) == 0) {
        return 0;
    } else {
        printf("chdir: %s: no such directory\n", path);
        return -1;
    }
}

/* ── MOSIX English: run / exec ───────────────────────────────────────────── */
static int cmd_run(int argc, char *argv[])
{
    if (argc < 2) {
        printf("run what? Usage: run <binary>\n");
        return -1;
    }

    int pid = getpid();
    printf("ZirvShell: execute '%s' as PID %d...\n", argv[1], pid);

    char *exec_argv[] = { argv[1], NULL };
    execve(argv[1], exec_argv, NULL);

    printf("execve: %s: command not found\n", argv[1]);
    return -1;
}

/* ── MOSIX English: create ───────────────────────────────────────────────── */
static int cmd_create(int argc, char *argv[])
{
    if (argc < 3) {
        printf("create what? Usage: create file <path>  or  create folder <path>\n");
        return -1;
    }
    if (strcmp(argv[1], "file") == 0) {
        int fd = open(argv[2], O_CREAT);
        if (fd < 0) {
            printf("create file: could not create '%s'\n", argv[2]);
            return -1;
        }
        close(fd);
        return 0;
    } else if (strcmp(argv[1], "folder") == 0) {
        if (mkdir(argv[2]) == 0) {
            return 0;
        } else {
            printf("create folder: could not create '%s'\n", argv[2]);
            return -1;
        }
    } else {
        printf("Unknown type '%s'. Use 'file' or 'folder'.\n", argv[1]);
        return -1;
    }
}

/* ── MOSIX English: delete / rm / rmdir / unlink ─────────────────────────── */
static int cmd_delete(int argc, char *argv[])
{
    if (argc < 3) {
        printf("delete what? Usage: delete file <path>  or  delete folder <path>\n");
        return -1;
    }
    if (strcmp(argv[1], "file") == 0) {
        if (unlink(argv[2]) == 0) {
            return 0;
        } else {
            printf("delete file: could not delete '%s'\n", argv[2]);
            return -1;
        }
    } else if (strcmp(argv[1], "folder") == 0) {
        if (rmdir(argv[2]) == 0) {
            return 0;
        } else {
            printf("delete folder: could not delete '%s' (directory may not be empty)\n", argv[2]);
            return -1;
        }
    } else {
        printf("Unknown type '%s'. Use 'file' or 'folder'.\n", argv[1]);
        return -1;
    }
}

/* ── rename / mv ─────────────────────────────────────────────────────────── */
static int cmd_rename(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: rename <old> <new>\n");
        return -1;
    }
    if (rename(argv[1], argv[2]) == 0) {
        return 0;
    } else {
        printf("rename: could not rename '%s' to '%s'\n", argv[1], argv[2]);
        return -1;
    }
}

/* ── copy / cp ───────────────────────────────────────────────────────────── */
static int cmd_copy(int argc, char *argv[])
{
    (void)argv;
    if (argc < 3) {
        printf("Usage: copy <src> <dst>\n");
        return -1;
    }
    printf("copy: not yet implemented\n");
    return -1;
}

/* ── mkdir POSIX ─────────────────────────────────────────────────────────── */
static int cmd_mkdir(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: mkdir <path>\n");
        return -1;
    }
    if (mkdir(argv[1]) == 0) {
        return 0;
    } else {
        printf("mkdir: could not create '%s'\n", argv[1]);
        return -1;
    }
}

/* ── rm / rmdir / unlink POSIX ──────────────────────────────────────────── */
static int cmd_rm(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: rm <path>\n");
        return -1;
    }
    if (unlink(argv[1]) == 0) {
        return 0;
    } else {
        printf("rm: could not remove '%s'\n", argv[1]);
        return -1;
    }
}

static int cmd_rmdir(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: rmdir <path>\n");
        return -1;
    }
    if (rmdir(argv[1]) == 0) {
        return 0;
    } else {
        printf("rmdir: could not remove '%s' (directory may not be empty)\n", argv[1]);
        return -1;
    }
}

/* ── mv POSIX ────────────────────────────────────────────────────────────── */
static int cmd_mv(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: mv <old> <new>\n");
        return -1;
    }
    if (rename(argv[1], argv[2]) == 0) {
        return 0;
    } else {
        printf("mv: could not rename '%s' to '%s'\n", argv[1], argv[2]);
        return -1;
    }
}

int main(void)
{
    printf("ZirvShell v2.0 (MOSIX Compliant)\n");
    printf("Lead Developer: Gautham Nair\n");
    printf("Type 'help' for MOSIX English syntax commands.\n");
    printf("POSIX/UNIX users: all familiar commands (ls, cd, mkdir, rm, mv, ...) also work.\n");

    char cmd[MAX_CMD_LEN];
    char *argv[MAX_ARGS];

    for (;;) {
        print_prompt();

        int n = read_line(cmd, sizeof(cmd));
        if (n <= 0) {
            printf("\n");
            continue;
        }

        int argc = parse_args(cmd, argv, MAX_ARGS);
        if (argc == 0) continue;

        /* ── Multi-word commands (must be checked before argv[0] only) ──── */

        /* "help me please" */
        if (argc >= 3 && strcmp(argv[0], "help") == 0
                      && strcmp(argv[1], "me") == 0
                      && strcmp(argv[2], "please") == 0) {
            cmd_help();

        /* "clear the screen" */
        } else if (argc >= 3 && strcmp(argv[0], "clear") == 0
                             && strcmp(argv[1], "the") == 0
                             && strcmp(argv[2], "screen") == 0) {
            cmd_clear();

        /* "go to <path>" */
        } else if (strcmp(argv[0], "go") == 0) {
            cmd_go(argc, argv);

        /* "run <binary>" */
        } else if (strcmp(argv[0], "run") == 0) {
            cmd_run(argc, argv);

        /* "create file/folder <path>" */
        } else if (strcmp(argv[0], "create") == 0) {
            cmd_create(argc, argv);

        /* "delete file/folder <path>" */
        } else if (strcmp(argv[0], "delete") == 0) {
            cmd_delete(argc, argv);

        /* "show files" / "show system status" */
        } else if (strcmp(argv[0], "show") == 0) {
            cmd_show(argc, argv);

        /* "copy <src> <dst>" */
        } else if (strcmp(argv[0], "copy") == 0) {
            cmd_copy(argc, argv);

        /* "rename <old> <new>" */
        } else if (strcmp(argv[0], "rename") == 0) {
            cmd_rename(argc, argv);

        /* ── Simple POSIX + English aliases ─────────────────────────────── */
        } else if (strcmp(argv[0], "help") == 0) {
            cmd_help();
        } else if (strcmp(argv[0], "echo") == 0 || strcmp(argv[0], "say") == 0) {
            cmd_echo(argc, argv);
        } else if (strcmp(argv[0], "pwd") == 0 || strcmp(argv[0], "where") == 0) {
            cmd_pwd();
        } else if (strcmp(argv[0], "uname") == 0) {
            cmd_uname();
        } else if (strcmp(argv[0], "clear") == 0) {
            cmd_clear();
        } else if (strcmp(argv[0], "exit") == 0) {
            printf("ZirvShell: exiting...\n");
            break;
        } else if (strcmp(argv[0], "whoami") == 0) {
            cmd_whoami();
        } else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "list") == 0) {
            cmd_ls(argc, argv);
        } else if (strcmp(argv[0], "cat") == 0 || strcmp(argv[0], "display") == 0) {
            cmd_cat(argc, argv);
        } else if (strcmp(argv[0], "date") == 0) {
            cmd_date();
        } else if (strcmp(argv[0], "settime") == 0) {
            cmd_settime(argc, argv);
        } else if (strcmp(argv[0], "uptime") == 0) {
            cmd_uptime();
        } else if (strcmp(argv[0], "timezone") == 0) {
            cmd_timezone(argc, argv);
        } else if (strcmp(argv[0], "reboot") == 0) {
            cmd_reboot();
        } else if (strcmp(argv[0], "shutdown") == 0 || strcmp(argv[0], "poweroff") == 0) {
            cmd_shutdown();
        } else if (strcmp(argv[0], "suspend") == 0) {
            cmd_suspend();
        } else if (strcmp(argv[0], "cd") == 0) {
            cmd_go(argc, argv);
        } else if (strcmp(argv[0], "exec") == 0) {
            cmd_run(argc, argv);
        } else if (strcmp(argv[0], "mkdir") == 0) {
            cmd_mkdir(argc, argv);
        } else if (strcmp(argv[0], "rm") == 0) {
            cmd_rm(argc, argv);
        } else if (strcmp(argv[0], "rmdir") == 0) {
            cmd_rmdir(argc, argv);
        } else if (strcmp(argv[0], "mv") == 0) {
            cmd_mv(argc, argv);
        } else if (strcmp(argv[0], "cp") == 0) {
            cmd_copy(argc, argv);
        } else {
            /* Look up the command in /bin/<name> automatically */
            char binpath[256];
            snprintf(binpath, sizeof(binpath), "/bin/%s", argv[0]);
            /* Reuse argv but point argv[0] at the binpath so the program
             * sees its own canonical path (conventional). */
            char *exec_argv[MAX_ARGS];
            exec_argv[0] = binpath;
            for (int i = 1; i < argc && i < MAX_ARGS - 1; i++)
                exec_argv[i] = argv[i];
            exec_argv[argc] = NULL;
            execve(binpath, exec_argv, NULL);
            /* execve only returns on failure */
            printf("ZirvShell: '%s': command not found. Type 'help'.\n", argv[0]);
        }
    }

    return 0;
}
