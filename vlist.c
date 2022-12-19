#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef enum {
    VT_REAL,
    VT_INTEGER,
    VT_STRING
} vtype;

static unsigned char *print_real(unsigned char *native)
{
    const char *fmt = "=%g\n";
    uint_least32_t value = native[4] | (native[3] << 8) | (native[2] << 16) | (native[1] << 24);
    if (value & 0x80000000)
        fmt = "=-%g\n";
    value |= 0x80000000;
    printf(fmt, (double)value * pow(2, native[0]-0x80-32));
    return native + 5;
}

static unsigned char *print_integer(unsigned char *native)
{
    uint_least32_t value = native[0] | (native[1] << 8) | (native[2] << 16) | (native[3] << 24);
    printf("=%d\n", value);
    return native + 4;
}

static unsigned char *print_string(unsigned char *base, unsigned char *native)
{
    uint_least16_t value = native[0] | (native[1] << 8);
    unsigned char *str = base + value;
    unsigned char *end = str + native[3];
    fputs("=\"", stdout);
    while (str < end) {
        int ch = *str++;
        if (ch == '"')
            putchar(ch);
        putchar(ch);
    }
    fputs("\"\n", stdout);
    return native + 4;
}

static unsigned char *print_value(unsigned char *base, unsigned char *native, vtype vt)
{
    switch(vt) {
        case VT_REAL:
            return print_real(native);
        case VT_INTEGER:
            return print_integer(native);
        case VT_STRING:
            return print_string(base, native);
        default:
            putchar('\n');
            return native;
    }
}

static unsigned char *dimension(unsigned char *base, unsigned char *buf, unsigned char *bufptr, unsigned char *data, vtype vt, uint_least8_t dims, unsigned char *dimptr)
{
    if (dims) {
        uint_least16_t dimsz = *dimptr++;
        dimsz |= (*dimptr++ << 8);
        for (int ix = 0; ix < dimsz; ++ix) {
            unsigned char *nxtbuf = bufptr + sprintf((char *)bufptr, "%d,", ix);
            data = dimension(base, buf, nxtbuf, data, vt, dims-1, dimptr);
        }
    }
    else {
        bufptr[-1] = ')';
        fwrite(buf, bufptr-buf, 1, stdout);
        data = print_value(base, data, vt);
    }
    return data;
}

static void print_var(int vname, unsigned char *base, unsigned char *native)
{
    vtype vt = VT_REAL;
    int ch;
    int array = 0;
    unsigned char *ptr = native;
    do {
        ch = *ptr++;
        if (ch == '%')
            vt = VT_INTEGER;
        else if (ch == '$')
            vt = VT_STRING;
        else if (ch == '(')
            array = 1;
    } while (ch);

    if (array) {
        uint_least8_t dims = ptr[0];
        unsigned char *data = ptr + dims;
        unsigned char buf[256];
        buf[0] = vname;
        size_t namelen = ptr-native;
        memcpy(buf+1, native, namelen);
        unsigned char *bufptr = buf + 1 + namelen;
        dims = (dims-1)/2;
        data = dimension(base, buf, bufptr, data, vt, dims, ptr+1);
    }
    else {
        putchar(vname);
        fwrite(native, ptr-native, 1, stdout);
        print_value(base, ptr, vt);
    }
}

static void read_err(const char *fn, FILE *fp)
{
    const char *msg = ferror(fp) ? strerror(errno) : "premature EOF";
    fprintf(stderr, "vlist: %s on %s\n", msg, fn);
}

static void vlist(const char *fn, FILE *fp)
{
    unsigned char page4[0xf6];
    if (fread(page4, sizeof(page4), 1, fp) == 1) {
        if (!memcmp(page4+0x70, "BBCBASVR", 8)) {
            unsigned char *ptr = page4;
            unsigned char *end = page4 + sizeof(page4);
            fputs("Resident Integer Variables\n", stdout);
            for (int vname = '@'; vname <= 'Z'; ++vname) {
                unsigned value = *ptr++;
                value |= (*ptr++) << 8;
                value |= (*ptr++) << 16;
                value |= (*ptr++) << 24;
                printf("%c%%=%08X (%d)\n", vname, value, value);
            }
            unsigned lomem = *ptr++;
            lomem |= (*ptr++) << 8;
            unsigned heapsz = *ptr++;
            heapsz |= (*ptr++) << 8;
            printf("LOMEM=%04X, heap size=%04X (%d)\n", lomem, heapsz, heapsz);
            unsigned char *heap = malloc(heapsz);
            if (heap) {
                if (fread(heap, heapsz, 1, fp) == 1) {
                    unsigned char *base = heap - lomem;
                    ptr = page4 + 0x80;
                    int vname = '@';
                    while (ptr < end) {
                        unsigned head = *ptr++;
                        head |= (*ptr++) << 8;
                        if (head) {
                            unsigned char *native = base + head;
                            print_var(vname, base, native+2);
                            unsigned char next_hi = native[1];
                            while (next_hi) {
                                unsigned next = native[0] | (next_hi << 8);
                                native = base + next;
                                print_var(vname, base, native+2);
                                next_hi = native[1];
                            }
                        }
                        ++vname;
                    }
                }
                else
                    read_err(fn, fp);
            }
            else
                fprintf(stderr, "vlist: unable to allocate %d bytes for BASIC heap: %m", heapsz);
        }
        else
            fprintf(stderr, "vlist: file %s is not a BBC BASIC variable dump\n", fn);
    }
    else
        read_err(fn, fp);
}

int main(int argc, char **argv)
{
    int status = 0;
    while (--argc) {
        const char *fn = *++argv;
        FILE *fp = fopen(fn, "rb");
        if (fp) {
            vlist(fn, fp);
            fclose(fp);
        }
        else {
            fprintf(stderr, "vlist: unable to open %s for reading: %m", fn);
            status = 1;
        }
    }
    return status;
}
