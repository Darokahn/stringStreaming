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
    uint32_t capacity;
    uint32_t length;
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
    uint8_t* result = bytestream_embed(s->base + s->length, data, s->capacity - s->length, s->align, size);
    if (result == NULL) {
        return 0;
    }
    s->length = result - s->base;
    return size;
}

static objectstream_t* objectstream_init(objectstream_t* s, uint8_t* base, uint32_t bufsize, uint8_t align) {
    if ((align & (align - 1)) != 0) {
        s->base = NULL;
        return NULL;
    }
    *s = (objectstream_t){.base=base, .capacity=bufsize, .length=0, .align=align};
    return s;
}

// TODO a bitstream which has two constructors; one for the exact bitsize of the objects it holds, and one for the number of distinct values that a slot might hold.
// accesses will use memcpy and bitwise math to return values from the array that line up with individual objects.
// A generic version of C++ std::vector<bool>

// TODO a macro that defines wrapper functions around `objectstream` for binding it to a fixed-size type with push/pop/at operations.
