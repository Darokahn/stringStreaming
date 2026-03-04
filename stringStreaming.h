#pragma once

#include <stdint.h>
#include <sys/param.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "interfaces.h"

// All types here are made so that they can be passed as a pair of pointers (base, stream function) and used by a caller who does not know their type.

#define heapstring_remainingoffset 1
#define heapstring_baseoffset (heapstring_remainingoffset + sizeof (uint32_t))
#define heapstring_basebindoffset (heapstring_baseoffset + sizeof (char*))

#define heapstring_minsize (sizeof (uint32_t) + sizeof (char*) + sizeof (char**) + 1)

#define staticstring_minsize (1 + sizeof (uint32_t))

static char** staticstring_init(char** s, uint32_t bufsize) {
    if (bufsize < staticstring_minsize) {
        return NULL;
    }
    memcpy(*s + 1, &bufsize, sizeof bufsize);
    return s;
}

static uint32_t staticstring_getRemaining(char* s) {
    uint32_t remaining;
    memcpy(&remaining, s + 1, sizeof remaining);
    return remaining;
}

static void staticstring_setRemaining(char* s, uint32_t remaining) {
    memcpy(s + 1, &remaining, sizeof remaining);
}

static int staticstring_stream(char** s, char* fmt, ...) {
    if (*s == NULL) return -1;
    va_list args;
    va_start(args, fmt);
    int remaining = staticstring_getRemaining(*s);
    int printed = vsnprintf(*s, remaining, fmt, args);
    int potentialRemaining = remaining - printed;
    if (potentialRemaining < staticstring_minsize) {
        // decommission the pointer
        *s = NULL;
        printed = -1;
        goto cleanup;
    }
    remaining = potentialRemaining;
    *s += printed;
    staticstring_setRemaining(*s, remaining);
cleanup:
    va_end(args);
    return printed;
}

// put necessary information at proper offsets
static void heapstring_serialize(char* s, uint32_t remaining, char* base, char** baseBinding) {
    memcpy(s + heapstring_remainingoffset, &remaining, sizeof remaining);
    memcpy(s + heapstring_baseoffset, &base, sizeof base);
    memcpy(s + heapstring_basebindoffset, &baseBinding, sizeof baseBinding);
}

// remaining characters
static int heapstring_getRemaining(char* s) {
    uint32_t remaining;
    memcpy(&remaining, s + heapstring_remainingoffset, sizeof remaining);
    return remaining;
}

// the beginning of the allocation
static char* heapstring_getBase(char* s) {
    char* base;
    memcpy(&base, s + heapstring_baseoffset, sizeof base);
    return base;
}

// the base char* this heap string is married to
static char** heapstring_getBaseBinding(char* s) {
    char** baseBinding;
    memcpy(&baseBinding, s + heapstring_basebindoffset, sizeof baseBinding);
    return baseBinding;
}

static void heapstring_bind(char* s, char** b) {
    int debug_basebindoffset = heapstring_basebindoffset;
    memcpy(s + heapstring_basebindoffset, &b, sizeof b);
    *b = heapstring_getBase(s);
}

static void heapstring_unbind(char* s) {
    *heapstring_getBaseBinding(s) = NULL;
    memset(s + heapstring_basebindoffset, 0, sizeof (char**));
}

static char** heapstring_init(char** s, uint32_t initialSize) {
    initialSize = MAX(initialSize, heapstring_minsize);
    *s = malloc(initialSize);
    if (*s == NULL) return NULL;
    heapstring_serialize(*s, initialSize, *s, 0);
    return s;
}

// always fully prints the string and returns that number, unless realloc fails
static int heapstring_stream(char** s, char* fmt, ...) {
    uint32_t remaining = heapstring_getRemaining(*s);
    char* base = heapstring_getBase(*s);
    char** baseBind = heapstring_getBaseBinding(*s);
    bool useBinding = baseBind != 0;
    va_list args;
    int printed;
    int stringSize;
    int potentialRemaining;
    while (true) {
        va_start(args, fmt);
        stringSize = vsnprintf(*s, remaining, fmt, args);
        printed = MIN(remaining, stringSize);
        potentialRemaining = remaining - printed;
        if (potentialRemaining < heapstring_minsize) {
            ptrdiff_t currentIndex = *s - base;
            uint32_t capacity = (currentIndex + stringSize + heapstring_minsize) * 2;
            char* oldBase = base;
            base = realloc(base, capacity);
            if (base == NULL) {
                base = oldBase;
                potentialRemaining = heapstring_minsize;
                printed = remaining - heapstring_minsize;
                goto cleanup;
            }
            *s = base + currentIndex;
            if (useBinding) {
                // we want to preserve any walking the bound base did, as long as it's validly inside the bounds
                ptrdiff_t walkDistance = *baseBind - oldBase;
                // if binding breaks its promise to stay in bounds, unbind it
                if (walkDistance < 0 || walkDistance > currentIndex) {
                    *baseBind = NULL;
                    heapstring_unbind(*s);
                    useBinding = false;
                }
                *baseBind = base + walkDistance;
            }
            remaining = capacity - currentIndex;
            va_end(args);
        }
        else break;
    }
cleanup:
    va_end(args);
    *s += printed;
    remaining = potentialRemaining;
    heapstring_serialize(*s, remaining, base, baseBind);
    return printed;
}

typedef struct linePrinter linePrinter;
struct linePrinter {
    int tabCount;
    char* newline;
    int newlineLen;
    char* indent;
    int indentLen;
    erased outDevice;
    aprintf printf;
    bool lineEdge;
};
    static int linePrinter_stream(linePrinter* t, char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char* allocated = NULL;
        int len = vasprintf(&allocated, fmt, args);
        int baseIndex = 0;
        char* proxy;
        int proxyLen;
        bool newline;
        bool tab;
        int totalPrinted = 0;
        for (int i = 0; i < len; i++) {
            if (t->lineEdge) {
                for (int j = 0; j < t->tabCount; j++) {
                    totalPrinted += t->printf(t->outDevice, "%.*s", t->indentLen, t->indent);
                }
                t->lineEdge = false;
            }
            newline = allocated[i] == '\n';
            tab = allocated[i] == '\t';
            if (newline) {
                proxy = t->newline;
                proxyLen = t->newlineLen;
                t->lineEdge = true;
            }
            else if (tab) {
                proxy = t->indent;
                proxyLen = t->indentLen;
            }
            else continue;
            totalPrinted += t->printf(t->outDevice, "%.*s", (i - baseIndex), allocated + baseIndex);
            int iterations = t->tabCount;
            totalPrinted += t->printf(t->outDevice, "%.*s", proxyLen, proxy);
            baseIndex = i + 1;
        }
        totalPrinted += t->printf(t->outDevice, "%.*s", len - baseIndex, allocated + baseIndex);
        free(allocated);
        return totalPrinted;
    }
    static linePrinter* linePrinter_init(linePrinter* t, char* newline, char* tabstr, erased outDevice, aprintf streamingFunc) {
        t->newline = newline;
        t->newlineLen = strlen(newline);
        t->indent = tabstr;
        t->indentLen = strlen(tabstr);
        t->tabCount = 0;
        t->outDevice = outDevice;
        t->printf = streamingFunc;
        t->lineEdge = true;
        return t;
    }

