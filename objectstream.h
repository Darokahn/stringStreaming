// a header for a vector of objects which might not all be the same size but must be aligned according to some alignment

#pragma once

#define alignof __alignof

#include <stdint.h>
#include <stddef.h>
#include <string.h>

static void* alignSize(void* ptr, uint8_t align) {
    intptr_t ref = (intptr_t)ptr;
    return (uint8_t*) ((ref + align - 1) & ~(align - 1));
}

#define alignType(ptr, type) ((type*) alignSize(ptr, alignof (type)))

typedef struct {
    uint8_t* base;
    uint32_t remaining;
    uint8_t align;
} objectstream_t;

static uint8_t* bytestream_embed(uint8_t* s, void* data, int remaining, uint8_t align, size_t size) {
    uint8_t* alignedPtr = alignSize(s, align);
    uint8_t* terminalPtr = alignedPtr + size;
    if (terminalPtr > s + remaining) {
        s = NULL;
        return NULL;
    }
    memmove(alignedPtr, data, size);
    return terminalPtr;
}

static int objectstream_write(objectstream_t *s, void* data, size_t size) {
    if (s->base == NULL) return -1;
    s->base = bytestream_embed(s->base, data, s->remaining, s->align, size);
    if (s->base == NULL) {
        return 0;
    }
    return size;
}

static objectstream_t* objectstream_init(objectstream_t* s, uint8_t* base, uint32_t bufsize, uint8_t align) {
    if ((align & (align - 1)) != 0) {
        s->base = NULL;
        return NULL;
    }
    *s = (objectstream_t){.base=base, .remaining=bufsize, .align=align};
    return s;
}
