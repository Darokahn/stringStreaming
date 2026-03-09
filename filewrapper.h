#pragma once

#include <poll.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>


bool wouldBlock(FILE* f, int events) {
    struct pollfd fd = {
        fileno(f), 
        events
    };
    poll(&fd, 1, 0);
    return events == fd.revents;
}

positive afprintf(FILE* f, char* fmt, ...) {
    va_list args; 
    va_start(args, fmt);
    int result = vfprintf(f, fmt, args);
    va_end(args);
    return result;
}

positive afwrite(FILE* f, char* buf, int len) {
    return fwrite(buf, 1, len, f);
}

positive afscanf(FILE* f, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vfscanf(f, fmt, args);
    va_end(args);
    return result;
}

positive afread(FILE* f, char* buf, int len) {
    return fread(buf, 1, len, f);
}

positive afgets(FILE*f, char* buf, int len) {
    char* val = fgets(buf, len, f);
    if (val == NULL) return EOF;
    return 0;
}

positive afgetc(FILE* f) {
    return fgetc(f);
}

positive afungetc(FILE* f, int c) {
    return ungetc(c, f);
}

positive aflen(FILE* f) {
    long current = ftell(f);
    if (current == -1L) return -1;

    if (fseek(f, 0, SEEK_END) != 0) return -1;
    long length = ftell(f);

    fseek(f, current, SEEK_SET);

    return length;
}

positive aftell(FILE* f) {
    return ftell(f);
}

positive afseek(FILE* f, int cursor) {
    return fseek(f, cursor, SEEK_SET);
}

positive afseekflags(FILE* f, int cursor, int flags) {
    return fseek(f, cursor, flags);
}

bool afsetblocking(FILE* f, bool allowed) {
    int fd = fileno(f);
    int flags = fcntl(fd, F_GETFL, 0);
    if (allowed) fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    else         fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return allowed;
}

read_vt fileread_vt = {.read=erase afread, .scan=erase fscanf, .getc= erase afgetc, .ungetc=erase afungetc, .gets =erase afgets};
seek_vt fileseek_vt = {.seek=erase afseek, .tell=erase ftell, .len=erase aflen};
write_vt filewrite_vt = {.write=erase afwrite, .printf=erase afprintf};
config_vt fileconfig_vt = {.setblocking=erase afsetblocking};

