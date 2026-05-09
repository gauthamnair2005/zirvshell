#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <zirv/syscall.h>

#define MAX_CMD_LEN 256
#define MAX_ARGS    16

static void print_prompt(void)
{
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) > 0) {
        printf("%s $ ", cwd);
    } else {
        printf("$ ");
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

static int cmd_help(void)
{
    printf("ZirvShell MOSIX Commands:\n");
    printf("  help                  Show this help\n");
    printf("  echo <text>           Print text\n");
    printf("  pwd                   Print working directory\n");
    printf("  uname                 Print system information\n");
    printf("  whoami                Show current user\n");
    printf("  clear                 Clear the screen\n");
    printf("  ls                    List directory contents\n");
    printf("  cat <file>            Display file contents\n");
    printf("  run <binary>          Execute a program\n");
    printf("  exec <binary>         Execute a program (replaces shell)\n");
    printf("  show files            List directory\n");
    printf("  show system status    Show system info\n");
    printf("  go to <path>          Change directory\n");
    printf("  date                  Print the system date/time\n");
    printf("  uptime                Print system uptime\n");
    printf("  reboot                Reboot the system\n");
    printf("  exit                  Exit the shell\n");
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

static int cmd_ls(void)
{
    printf("/bin/   System binaries\n");
    printf("/zirv/  Device namespace\n");
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

static int cmd_date(void)
{
    printf("date: not available\n");
    return 0;
}

static int cmd_uptime(void)
{
    printf("uptime: not available\n");
    return 0;
}

static int cmd_reboot(void)
{
    printf("reboot: not available\n");
    return 0;
}

static int cmd_show(int argc, char *argv[])
{
    if (argc < 2) {
        printf("show what? Try: show files, show system status\n");
        return -1;
    }
    if (strcmp(argv[1], "files") == 0) {
        printf("Directory listing:\n");
        printf("  /bin/    - System binaries\n");
        printf("  /zirv/   - Device namespace\n");
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

static int cmd_go(int argc, char *argv[])
{
    if (argc < 3) {
        printf("go to where? Usage: go to <path>\n");
        return -1;
    }
    if (chdir(argv[2]) == 0) {
        return 0;
    } else {
        printf("chdir: %s: no such directory\n", argv[2]);
        return -1;
    }
}

static int cmd_run(int argc, char *argv[])
{
    if (argc < 2) {
        printf("run what? Usage: run <binary>\n");
        return -1;
    }

    int pid = getpid();
    printf("ZirvShell: execute '%s' as PID %d...\n", argv[0], pid);

    char *exec_argv[] = { argv[1], NULL };
    execve(argv[1], exec_argv, NULL);

    printf("execve: %s: command not found\n", argv[1]);
    return -1;
}

int main(void)
{
    printf("ZirvShell v1.0 (MOSIX Compliant)\n");
    printf("Lead Developer: Gautham Nair\n");
    printf("Type 'help' for MOSIX English syntax commands.\n");

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

        if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "help me please") == 0) {
            cmd_help();
        } else if (strcmp(argv[0], "echo") == 0 || strcmp(argv[0], "say") == 0) {
            cmd_echo(argc, argv);
        } else if (strcmp(argv[0], "pwd") == 0 || strcmp(argv[0], "where") == 0) {
            cmd_pwd();
        } else if (strcmp(argv[0], "uname") == 0) {
            cmd_uname();
        } else if (strcmp(argv[0], "clear") == 0 || strcmp(argv[0], "clear the screen") == 0) {
            cmd_clear();
        } else if (strcmp(argv[0], "exit") == 0) {
            printf("ZirvShell: exiting...\n");
            break;
        } else if (strcmp(argv[0], "whoami") == 0) {
            cmd_whoami();
        } else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "list") == 0) {
            cmd_ls();
        } else if (strcmp(argv[0], "cat") == 0 || strcmp(argv[0], "display") == 0) {
            cmd_cat(argc, argv);
        } else if (strcmp(argv[0], "date") == 0) {
            cmd_date();
        } else if (strcmp(argv[0], "uptime") == 0) {
            cmd_uptime();
        } else if (strcmp(argv[0], "reboot") == 0) {
            cmd_reboot();
        } else if (strcmp(argv[0], "show") == 0) {
            cmd_show(argc, argv);
        } else if (strcmp(argv[0], "go") == 0 || strcmp(argv[0], "cd") == 0) {
            cmd_go(argc, argv);
        } else if (strcmp(argv[0], "run") == 0 || strcmp(argv[0], "exec") == 0) {
            cmd_run(argc, argv);
        } else {
            printf("ZirvShell: '%s': unknown command. Type 'help'.\n", argv[0]);
        }
    }

    return 0;
}
