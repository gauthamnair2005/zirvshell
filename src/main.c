#include <stdio.h>
#include <string.h>
#include <zirv/syscall.h>

/* ZirvShell — MOSIX Modern English Shell
 * Syntax:
 *   "show files"             -> list directory
 *   "go to <path>"          -> change directory
 *   "run <binary>"          -> execute
 *   "display <file>"        -> cat/type
 *   "create folder <name>"  -> mkdir
 */

void print_prompt() {
    printf("\nZirvShell > ");
}

int main() {
    printf("ZirvShell v1.0 (MOSIX Compliant)\n");
    printf("Lead Developer: Gautham Nair\n");
    printf("Type 'help' for MOSIX English syntax commands.\n");

    char cmd[256];
    for (;;) {
        print_prompt();
        
        /* TODO: implement actual line reading via SYS_READ */
        /* For now, just a stub for syntax demonstration */
        printf("Stub: Waiting for MOSIX input...\n");
        break;
    }

    return 0;
}
