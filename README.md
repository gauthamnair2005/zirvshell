# zirvshell — MOSIX Interactive Shell (Reference Shell)

Command-line shell for **MOSIX** operating systems. Supports English-like
natural language syntax alongside standard POSIX command names.

Part of the [Zirvium](https://github.com/gauthamnair2005/zirvium) reference
MOSIX implementation. See the [MOSIX specification](https://github.com/gauthamnair2005/zirvworld)
for the full standard.

## Commands (19 total)

| Command | Aliases | Description |
|---------|---------|-------------|
| `help` | `help me please` | Show help text |
| `echo` | `say` | Print text |
| `pwd` | `where` | Print working directory |
| `uname` | | Print kernel name |
| `whoami` | | Show current user |
| `clear` | `clear the screen` | Clear screen |
| `ls` | `list`, `show files` | Directory listing via `getdents` |
| `cat` | `display` | Display file contents |
| `run` | `exec` | Execute a binary via `execve` |
| `go to` | `cd` | Change directory |
| `date` | | Print date/time |
| `settime` | | Set date/time |
| `timezone` | | Show/set timezone |
| `uptime` | | System uptime |
| `reboot` | | Reboot via SYS_REBOOT |
| `shutdown` | `poweroff` | Shutdown via SYS_SHUTDOWN |
| `suspend` | | Suspend (stub) |
| `exit` | | Exit shell |

## Features

- CWD-aware prompt (`/ $`)
- Line editor with backspace
- Double-quoted argument parsing
- Multi-word natural language: `help me please`, `clear the screen`

## Build

```bash
make
```

Produces `zirvshell.elf` — static, freestanding, no-pie ELF64.
Linked against [zirvlibc](https://github.com/gauthamnair2005/zirvlibc).
