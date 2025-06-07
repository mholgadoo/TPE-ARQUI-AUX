#include "shell.h"
#include "syscalls.h"
#include <stdint.h>
#include <stddef.h>

// Defino numeros de syscall segun mi dispatcher
#define SYS_WRITE             0
#define SYS_READ              1
#define SYS_CLEAR_SCREEN      2
#define SYS_GET_REGS          6
#define SYS_GET_TIME          7
#define SYS_CHANGE_FONT_SIZE  9

static char *username;

/*
 * setUsername:
 * Asigno el nombre de usuario para el prompt.
 * Si name es NULL o cadena vacia uso "User".
 */
void setUsername(const char *name) {
    if (name == NULL || name[0] == '\0') {
        username = "User";
    } else {
        username = (char *)name;
    }
}

/*
 * str_eq:
 * Comparo dos cadenas caracter a caracter.
 * Retorno 1 si son identicas, 0 si no.
 */
static int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) {
        i++;
    }
    return a[i] == b[i];
}

/*
 * str_len:
 * Devuelve la longitud de una cadena terminada en '\0'
 */
static int str_len(const char *s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

/*
 * shell_print_help:
 * Muestro la lista de comandos disponibles.
 */
void shell_print_help() {
    _sys_write(SYS_WRITE,
        "Available commands:\n", 
        str_len("Available commands:\n"));
    _sys_write(SYS_WRITE,
        "help       - show this help\n", 
        str_len("help       - show this help\n"));
    _sys_write(SYS_WRITE,
        "divzero    - trigger division by zero\n", 
        str_len("divzero    - trigger division by zero\n"));
    _sys_write(SYS_WRITE,
        "invopcode  - trigger invalid opcode\n", 
        str_len("invopcode  - trigger invalid opcode\n"));
    _sys_write(SYS_WRITE,
        "time       - display system time\n", 
        str_len("time       - display system time\n"));
    _sys_write(SYS_WRITE,
        "regs       - display CPU registers\n", 
        str_len("regs       - display CPU registers\n"));
    _sys_write(SYS_WRITE,
        "clear      - clear screen\n", 
        str_len("clear      - clear screen\n"));
    _sys_write(SYS_WRITE,
        "font+      - increase font size\n", 
        str_len("font+      - increase font size\n"));
    _sys_write(SYS_WRITE,
        "font-      - decrease font size\n", 
        str_len("font-      - decrease font size\n"));
}

/*
 * read_line:
 * Leo caracteres usando _sys_read hasta '\\n' o max-1.
 * Soporto backspace '\\b'. Al final agrego '\\0'.
 */
static int read_line(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c;
        _sys_read(SYS_READ, 0, &c, 1);
        if (c == '\n') {
            break;
        } else if (c == '\b') {
            if (i > 0) i--;
        } else {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return i;
}

/*
 * trigger_divzero:
 * Provoco division por cero para generar la excepcion.
 */
static void trigger_divzero() {
    volatile int a = 1 / 0;
    (void)a;
}

/*
 * trigger_invopcode:
 * Provoco invalid opcode con la instruccion ud2.
 */
static void trigger_invopcode() {
    __asm__ __volatile__("ud2");
}

/*
 * print_time:
 * Pido la hora al kernel y la formateo como HH:MM:SS.
 */
static void print_time() {
    rtc_time_t tm;
    _sys_get_time(SYS_GET_TIME, &tm);

    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = '\0';
    buf[0] = '0' + (tm.hour / 10);
    buf[1] = '0' + (tm.hour % 10);
    buf[3] = '0' + (tm.min  / 10);
    buf[4] = '0' + (tm.min  % 10);
    buf[6] = '0' + (tm.sec  / 10);
    buf[7] = '0' + (tm.sec  % 10);

    _sys_write(SYS_WRITE, buf, 8);
    _sys_write(SYS_WRITE, "\n", 1);
}

/*
 * print_regs:
 * Pido registros al kernel y los imprimo en hexadecimal.
 */
static void print_regs() {
    uint64_t regs[17];
    _sys_get_registers(SYS_GET_REGS, regs);

    for (int i = 0; i < 17; i++) {
        char hex[16];
        uint64_t val = regs[i];
        for (int j = 15; j >= 0; j--) {
            int d = val & 0xF;
            hex[j] = d < 10 ? '0' + d : 'A' + (d - 10);
            val >>= 4;
        }
        _sys_write(SYS_WRITE, "0x", 2);
        _sys_write(SYS_WRITE, hex, 16);
        _sys_write(SYS_WRITE, "\n", 1);
    }
}

/*
 * shell_run:
 * Bucle principal de la shell.
 */
void shell_run() {
    char line[64];
    while (1) {
        _sys_write(SYS_WRITE, "@", 1);
        _sys_write(SYS_WRITE, username, str_len(username));
        _sys_write(SYS_WRITE, "$> ", 3);

        read_line(line, sizeof(line));
        if (line[0] == '\0') continue;

        if      (str_eq(line, "help"))      shell_print_help();
        else if (str_eq(line, "divzero"))   trigger_divzero();
        else if (str_eq(line, "invopcode")) trigger_invopcode();
        else if (str_eq(line, "time"))      print_time();
        else if (str_eq(line, "regs"))      print_regs();
        else if (str_eq(line, "clear"))     _sys_clearScreen(SYS_CLEAR_SCREEN);
        else if (str_eq(line, "font+"))     _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, +1);
        else if (str_eq(line, "font-"))     _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, -1);
        else _sys_write(SYS_WRITE, "Unknown command\n",
                        str_len("Unknown command\n"));
    }
}
