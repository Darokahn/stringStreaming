#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void* erased;
#define erase (void*)(intptr_t)

// The new standard `ascanf` and `aprintf` should recognize one extra formatter, `%a`, whose meaning is to take an `ioDevice_i` and write to it or read from it instead of a string.
// To be compatible with the new generic implementations of stdio funcs and generally to be compatible with one another, all `ascanf` and `aprintf` implementations must conform to:
// - recognize the full set of format verbs and always consume the right number of arguments for each one (no necessity to use them after consumption).
// - if a given string is split by complete format verbs and passed across several calls, the behavior is the same as with only one call.

typedef long long positive;

typedef positive (*aprintf)(erased ioDevice, char* fmt, ...);
typedef positive (*awrite)(erased ioDevice, char* input, size_t size);
typedef positive (*ascanf)(erased ioDevice, char* fmt);
typedef positive (*aread)(erased ioDevice, char* input, size_t size);
typedef positive (*agets)(erased ioDevice, char* dest, int max);
typedef positive (*agetc)(erased iodevice);
typedef positive (*aungetc)(erased iodevice, int c);
typedef positive (*alen)(erased ioDevice);
typedef positive (*atell)(erased ioDevice);
typedef positive (*aseek)(erased ioDevice, int cursor);
typedef positive (*aseekflags)(erased ioDevice, int cursor, int flags);
typedef bool (*asetblocking)(erased ioDevice, bool allowed);

/* suffixes:
 * _t       : type
 * _vt      : virtual table
 * _i       : interface
*/

// virtual tables may implement any set of the entries specified. Callers and callees must coordinate responsibility for checking whether a vtable includes an entry (!= NULL). For correctness and safety, it is a callee concern. For performance, it is a caller concern.

typedef struct {
    awrite write;
    aprintf printf;
} write_vt;

typedef struct {
    alen len;
    atell tell;
    aseek seek;
    aseekflags seekflags;
} seek_vt;

typedef struct {
    aread read;
    ascanf scan;
    agets gets;
    agetc getc;
    aungetc ungetc;
} read_vt;

typedef struct {
    asetblocking setblocking;
} config_vt;

typedef struct {
    erased base;
    write_vt* write;
} write_i;

typedef struct {
    erased base;
    read_vt* read;
} read_i;

typedef struct {
    erased readBase;
    erased writeBase;
    write_vt* write;
    read_vt* read;
    seek_vt* seek;
    config_vt* config;
} ioDevice_i;

typedef struct {
    erased base;
    read_vt* read;
    seek_vt* seek;
} readseek_i;

typedef struct {
    erased base;
    read_vt* read;
    seek_vt* seek;
} writeseek_i;

// TODO definitions for new standard versions of each FILE* function that instead takes an `ioDevice_i`
