#include "shell.h"
#include <stdint.h>
#include <syscall.h>
#include <stddef.h>

// puntero al nombre de usuario que se muestra en el prompt
static char * username;

/*
 * setUsername:
 * Asigna el nombre de usuario que se mostrará en el prompt. Si name es NULL
 * o cadena vacía, se usa "User" por defecto. En caso contrario, apunta a name.
 */
void setUsername(const char *name) {
    if (name == NULL || name[0] == 0) {
        username = "User";
    } else {
        username = name;
    }
}

/*
 * str_eq:
 * Compara dos cadenas carácter a carácter. Devuelve 1 si ambas son idénticas,
 * 0 si difieren. Se detiene al encontrar un carácter distinto o el terminador '\0'.
 */
static int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i])
        i++;
    return a[i] == b[i];
}

/*
 * print:
 * Imprime cada carácter de la cadena str usando syscall_write. Recorre str
 * hasta encontrar el terminador '\0'.
 */
static void print(const char *str) {
    for (int i = 0; str[i]; i++)
        syscall_write(str[i]);
}

/*
 * shell_print_help:
 * Muestra en pantalla la lista de comandos disponibles. Llama a print() para
 * cada línea de ayuda, incluyendo comandos básicos y comandos para cambiar
 * el tamaño de fuente.
 */
void shell_print_help() {
    print("Available commands:\n");
    print("help - show this help\n");
    print("divzero - trigger division by zero\n");
    print("invopcode - trigger invalid opcode\n");
    print("time - display system time\n");
    print("regs - display CPU registers\n");
    print("clear - clear screen\n");
    print("font+ - increase font size\n");
    print("font- - decrease font size\n");
}

/*
 * read_line:
 * Lee caracteres uno a uno usando syscall_read hasta '\n' o hasta max-1.
 * Soporta backspace '\b' para borrar el último carácter leido. Al final
 * agrega '\0' y devuelve la longitud de la línea leída.
 */
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

/*
 * trigger_divzero:
 * Provoca una excepción de división por cero. Se usa para que el kernel
 * capture esta interrupción, muestre los registros y luego regrese a la shell.
 */
static void trigger_divzero() {
    volatile int a = 1 / 0;
    (void)a;
}

/*
 * trigger_invopcode:
 * Ejecuta la instrucción inválida "ud2" para provocar una excepción de
 * invalid opcode. El kernel debe atraparla y luego retornar a la shell.
 */
static void trigger_invopcode() {
    __asm__ __volatile__("ud2");
}

/*
 * print_time:
 * Solicita al kernel la hora actual con syscall_get_time, formatea la hora
 * como "HH:MM:SS" y la imprime con print(). Añade un salto de línea al final.
 */
static void print_time() {
    int h, m, s;
    syscall_get_time(&h, &m, &s);
    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = 0;
    buf[0] = '0' + (h / 10); buf[1] = '0' + (h % 10);
    buf[3] = '0' + (m / 10); buf[4] = '0' + (m % 10);
    buf[6] = '0' + (s / 10); buf[7] = '0' + (s % 10);
    print(buf);
    print("\n");
}

/*
 * print_regs:
 * Pide al kernel todos los registros de CPU con syscall_get_regs, convierte
 * cada valor a una cadena hexadecimal de 16 dígitos y la imprime precedida
 * por "0x". Cada registro se muestra en su propia línea.
 */
static void print_regs() {
    unsigned long regs[17];
    syscall_get_regs(regs);
    for (int i = 0; i < 17; i++) {
        unsigned long val = regs[i];
        char hex[17];
        for (int j = 15; j >= 0; j--) {
            int digit = val & 0xF;
            hex[j] = digit < 10 ? '0' + digit : 'A' + digit - 10;
            val >>= 4;
        }
        hex[16] = 0;
        print("0x");
        for (int j = 0; j < 16; j++)
            syscall_write(hex[j]);
        print("\n");
    }
}

/*
 * shell_run:
 * Bucle principal de la shell. Repetidamente imprime el prompt "$> ", lee una
 * línea de comando en line y compara la cadena con los comandos conocidos:
 * "help", "divzero", "invopcode", "time", "regs", "clear", "font+", "font-".
 * Dependiendo del comando, llama a la función correspondiente. Si la línea
 * está vacía, vuelve a mostrar el prompt. Si el comando no se reconoce,
 * imprime "Unknown command\n".
 */
void shell_run() {
    char line[64];
    while (1) {        
        // imprime prompt con nombre de usuario: "@M&M$> "
        print("@");
        print(username);
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
        }
        // comando para cambiar tamaño de la fuente
        else if (str_eq(line, "font+")) {
            syscall_set_font_size(1);
        } else if (str_eq(line, "font-")) {
            syscall_set_font_size(-1);
        } else {
            print("Unknown command\n");
        }
    }
}
