#pragma once

typedef void* erased;
#define erase (void*)

typedef int (*aprintf)(erased ioDevice, char* fmt, ...);
typedef int (*awrite)(erased ioDevice, char* input, size_t size);
typedef int (*ascanf)(erased ioDevice, char* fmt, erased handlerDevice, awrite handlerFunc);
typedef int (*aread)(erased ioDevice, char* input, size_t size);
typedef int (*aseek)(erased ioDevice, int cursor);
typedef int (*atell)(erased ioDevice);

typedef struct {
    aprintf printf;
} streamable_vt;

typedef struct {
    aprintf printf;
    awrite write;
    aseek setCursor;
    atell getCursor;
} writable_vt;

typedef struct {
    ascanf scan;
    aread read;
} readable_vt;

typedef struct {
    writable_vt write;
    readable_vt read;
} ioDevice_vt;

