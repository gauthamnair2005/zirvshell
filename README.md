# zirvshell — MOSIX Interactive Shell

Command-line shell for the Zirvium kernel. Supports MOSIX English-like syntax alongside standard POSIX command names.

## Commands

| Command | Description |
|---------|-------------|
| `help` | Show help text |
| `echo`, `say` | Print text |
| `pwd`, `where` | Print working directory |
| `uname` | Print kernel name |
| `whoami` | Show current user |
| `clear` | Clear the screen |
| `ls`, `list` | List directory contents (stub) |
| `cat`, `display` | Display file contents (stub) |
| `run`, `exec` | Execute an embedded binary |
| `show files` | List directory |
| `show system status` | Show system info |
| `go to`, `cd` | Change directory |
| `date` | Print system date/time (stub) |
| `uptime` | Print system uptime (stub) |
| `reboot` | Reboot (stub) |
| `exit` | Exit the shell |

## Build

```bash
make
```

Produces `zirvshell.elf` — a statically linked, freestanding, no-pie ELF binary embedded into the kernel at `/bin/shell`.
