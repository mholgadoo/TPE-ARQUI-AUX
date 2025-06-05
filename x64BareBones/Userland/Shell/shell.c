#include "shell.h"
#include <stdint.h>
#include <syscall.h>
#include <stddef.h>



static char username[16] = "user";

void setUsername(const char *name) {
    if (!name)
        return;
    for (size_t i = 0; i < sizeof(username) - 1 && name[i]; i++) {
        username[i] = name[i];
        username[i + 1] = '\0';
    }
}

static int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i])
        i++;
    return a[i] == b[i];
}

static void print(const char *str) {
    for (int i = 0; str[i]; i++)
        syscall_write(str[i]);
}

void shell_print_help() {
    print("Available commands:\n");
    print("help - show this help\n");
    print("divzero - trigger division by zero\n");
    print("invopcode - trigger invalid opcode\n");
    print("time - display system time\n");
    print("regs - display CPU registers\n");
    print("clear - clear screen\n");
}

static int read_line(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = syscall_read();
        if (c == '\n') {
            break;
        } else if (c == '\b') {
            if (i > 0) i--;
        } else {
            buf[i++] = c;
        }
    }
    buf[i] = 0;
    return i;
}

static void trigger_divzero() {
    volatile int a = 1 / 0;
    (void)a;
}

static void trigger_invopcode() {
    __asm__ __volatile__("ud2");
}

static void print_time() {
    int h, m, s;
    syscall_get_time(&h, &m, &s);
    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = 0;
    buf[0] = '0' + (h / 10); buf[1] = '0' + (h % 10);
    buf[3] = '0' + (m / 10); buf[4] = '0' + (m % 10);
    buf[6] = '0' + (s / 10); buf[7] = '0' + (s % 10);
    print(buf); print("\n");
}

static void print_regs() {
    unsigned long regs[17];
    syscall_get_regs(regs);
    for(int i=0;i<17;i++) {
        unsigned long val = regs[i];
        char hex[17];
        for(int j=15;j>=0;j--) {
            int digit = val & 0xF;
            hex[j] = digit < 10 ? '0'+digit : 'A'+digit-10;
            val >>= 4;
        }
        hex[16] = 0;
        print("0x");
        for(int j=0;j<16;j++) syscall_write(hex[j]);
        print("\n");
    }
}

void shell_run() {
    char line[64];
    while (1) {
        print("$> ");
        read_line(line, sizeof(line));
        if (line[0] == 0) continue;
        if (str_eq(line, "help")) {
            shell_print_help();
        } else if (str_eq(line, "divzero")) {
            trigger_divzero();
        } else if (str_eq(line, "invopcode")) {
            trigger_invopcode();
        } else if (str_eq(line, "time")) {
            print_time();
        } else if (str_eq(line, "regs")) {
            print_regs();
        } else if (str_eq(line, "clear")) {
            syscall_clear_screen();
        } else {
            print("Unknown command\n");
        }
    }
}
int main() {
    shell_run();
    return 0;
}
