#include <kernel/snowflake.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

// string functions

void str_cat(char *s1, char *s2) {
    char *start = s1;
    while(*start != '\0') start++;
    while(*s2 != '\0') *start++ = *s2++;
    *start = '\0';
}

int str_len(char *s) {
    int i = 0;
    while (s[i] != '\0') i++;
    return i;
}

void str_cpy(char *s1, char *s2) {
    int i;
    for (i = 0; s2[i] != '\0'; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

void  str_ncpy(char *s1, char *s2, int n) {
    int i;
    for (i = 0; i < n && s2[i] != '\0'; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

void str_reverse(char *s) {
    int i = 0;
    int j = str_len(s) - 1;
    char tmp;
    while (i < j) {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}

void int2str(int n, char *s) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) s[i++] = '-';
    s[i] = '\0';

    str_reverse(s);
}

void hex2str(uint32_t n, char *s) {
    int i = 0;
    int tmp;
    char hex[] = "0123456789abcdef";
    do {
        tmp = n % 16;
        s[i++] = hex[tmp];
    } while ((n /= 16) > 0);
    s[i] = 'x';
    s[i+1] = '0';
    s[i+2] = '\0';
    str_reverse(s);
}

int str2int(char *s) {
    int i = 0;
    int n = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        n = 10 * n + (s[i++] - '0');
    }
    return n;
}

int str_cmp(char *s1, char *s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return s1[i] - s2[i];
}

int str_ncmp(char *s1, char *s2, int n) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (i == n || s1[i] == '\0') return 0;
    }
    if (i == n) return 0;
    return s1[i] - s2[i];
}

int str_count(char *s, char c) {
    int i = 0;
    int count = 0;
    while (s[i] != '\0') {
        if (s[i] == c) count++;
        i++;
    }
    return count;
}

void str_append(char *s, char c) {
    int i = 0;
    while (s[i] != '\0') i++;
    s[i] = c;
    s[i+1] = '\0';
}

// memory management

void mem_copy(void *dest, void *source, int nbytes) {
    for (int i = 0; i < nbytes; i++)
        *((uint8_t *) dest + i) = *((uint8_t *) source + i);
}

void mem_set(void *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = dest;
    for ( ; len != 0; len--)
        *temp++ = val;
}

void free(void *addr) {
    if (addr == NULL) return;
    int size = mem_get_alloc_size((uint32_t) addr);
    if (size == 0) {
        kprintf("kernel free: %x not allocated\n", addr);
        return;
    }

    mem_set((uint8_t *) addr, 0, size);
    mem_free_addr((int) addr);
}

void *malloc(uint32_t size) {
    uint32_t addr = mem_alloc(size, 0, 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc_as_kernel(void *ptr, uint32_t size) {
    uint32_t addr = (uint32_t) ptr;
    uint32_t new_addr = mem_alloc(size, 0, 6);
    if (new_addr == 0) return NULL;
    if (addr == 0) return (void *) new_addr;
    mem_copy((uint8_t *) new_addr, (uint8_t *) addr, size);
    mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(uint32_t size) {
    int addr = mem_alloc(size, 0, 1);
    if (addr == 0) return NULL;
    mem_set((uint8_t *) addr, 0, size);
    return (void *) addr;
}

// input/output functions

void status_print(int (*func)(), char *verb, char *noun) {
    int old_cursor, new_cursor, status;

    kcprint("[", 0x0F);
    old_cursor = get_cursor_offset();
    kcprint("WORK", 0x0E);
    kcprint("]  ", 0x0F);
    kcprint(verb, 0x07);
    kcprint(" ", 0x0F);
    kcprint(noun, 0x0F);
    kcprint("\n", 0x0F);

    status = func();
    new_cursor = get_cursor_offset();
    set_cursor_offset(old_cursor);

    if (status == 0) {
        kcprint(" OK ", 0x0A);
    } else if (status == 2) {
        kcprint("ENBL", 0x0B);
    } else {
        kcprint("FAIL", 0x0C);
    }

    set_cursor_offset(new_cursor);
}

#define LSHIFT  42
#define RSHIFT  54
#define BACK    14
#define ENTER   28
#define RESEND  224
#define SLEEP_T 15
#define FIRST_L 12
#define SC_MAX  57

void kinput(char *buffer, int size) {
    kprint("\e[?25l");

    int sc, last_sc, last_sc_sgt = 0;
    int index = 0;

    for (int i = 0; i < size; i++)
        buffer[i] = '\0';

    int key_ticks = 0;
    int shift = 0;
    char c;

    sc = last_sc = 0;
    while (sc != ENTER) {
        process_sleep(process_get_pid(), SLEEP_T);
        sc = kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        } else if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
        } else if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
        } else if (sc == BACK) {
            if (index == 0) continue;
            buffer[index] = '\0';
            index--;
            kprint("\e[1D \e[1D");
        } else if (sc <= SC_MAX) {
            if (size < index + 2) continue;
            c = kb_scancode_to_char(sc, shift);
            if (c == '\0') continue;
            kcnprint(&c, 1, c_blue);
            buffer[index] = c;
            index++;
        }
    }

    buffer[index] = '\0';
    kprint("\e[?25h");
}

void kprintf_va2buf(char *char_buffer, char *fmt, va_list args) {
    int output, buffer_i, i;
    output = buffer_i = i = 0;
    char s[12];

    if ((uint32_t) char_buffer < 2) {
        output = (int) char_buffer + 1;
        char_buffer = sys_safe_buffer;
    }

    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                char *tmp = va_arg(args, char *);
                for (int j = 0; tmp[j] != '\0'; j++) {
                    char_buffer[buffer_i] = tmp[j];
                    buffer_i++;
                }
            } else if (fmt[i] == 'c') {
                char_buffer[buffer_i] = va_arg(args, int);
                buffer_i++;
            } else if (fmt[i] == 'd') {
                int2str(va_arg(args, int), s);
                for (int j = 0; s[j] != '\0'; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == 'x') {
                hex2str(va_arg(args, int), s);
                for (int j = 0; s[j] != '\0'; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == '%') {
                char_buffer[buffer_i] = '%';
                buffer_i++;
            }
        } else {
            char_buffer[buffer_i] = fmt[i];
            buffer_i++;
        }
        i++;
    }
    char_buffer[buffer_i] = '\0';
    if (output == 1) {
        kprint(char_buffer);
    } else if (output == 2) {
        serial_print(SERIAL_PORT_A, char_buffer);
    }
}

void kprintf_buf(char *char_buffer, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kprintf_va2buf(char_buffer, fmt, args);
    va_end(args);
}

void krainbow(char *message) {
    char rainbow_colors[] = {c_green, c_cyan, c_blue, c_magenta, c_red, c_yellow};

    for (int i = 0; message[i]; i++) {
        kcnprint(&message[i], 1, rainbow_colors[i % 6]);
    }
}
