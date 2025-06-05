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
    if(hours) *hours = h;
    if(mins) *mins = m;
    if(secs) *secs = s;
}

static inline void syscall_get_regs(unsigned long *regs) {
    __asm__ __volatile__("int $0x80" : : "a"(4), "b"(regs));
}

#endif // SYSCALL_H
