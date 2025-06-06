#ifndef SYSCALL_H
#define SYSCALL_H

static inline void syscall_write(char c) {
    __asm__ __volatile__("int $0x80" : : "a"(1), "b"(c));
}

static inline char syscall_read() {
    char c;
    __asm__ __volatile__("int $0x80" : "=a"(c) : "a"(0));
    return c;
}

static inline void syscall_clear_screen() {
    __asm__ __volatile__("int $0x80" : : "a"(2));
}

static inline void syscall_get_time(int *hours, int *mins, int *secs) {
    int h, m, s;
    __asm__ __volatile__("int $0x80" : "=a"(h), "=b"(m), "=c"(s) : "a"(3));
    if (hours) *hours = h;
    if (mins)   *mins   = m;
    if (secs)   *secs   = s;
}

static inline void syscall_get_regs(unsigned long *regs) {
    __asm__ __volatile__("int $0x80" : : "a"(4), "b"(regs));
}

/*
 * syscall_set_font_size:
 *   Ajusta el tamaño de la fuente en pantalla. El parámetro delta puede
 *   ser positivo (incrementar tamaño) o negativo (reducir tamaño).
 *   Se invoca la interrupción 0x80 con eax=5 y ebx=delta.
 */
static inline void syscall_set_font_size(int8_t delta) {
    __asm__ __volatile__("int $0x80" : : "a"(5), "b"(delta));
}

#endif // SYSCALL_H
