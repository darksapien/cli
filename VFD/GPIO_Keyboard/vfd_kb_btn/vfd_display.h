//===============================
// FILE: vfd_display.h
//===============================
#ifndef VFD_DISPLAY_H
#define VFD_DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#define VFD_ROWS 6
#define VFD_COLS 40
#define VFD_CLEAR "\x19\x0C"
#define VFD_HOME "\x19\x0E"
#define VFD_BLINK_START "\x0B"
#define VFD_BLINK_END "\x0C"
#define VFD_BRIGHT "\x19\x4F"
#define VFD_SCROLL "\x10"
#define VFD_NEWLINE "\x0D\x0A"
#define VFD_CURSOR_UP "\x19\x0B"
#define VFD_CURSOR_POS "\x19\x11"
#define VFD_QUERY "\x04\x00"

typedef struct {
    int fd;
    struct termios old_tio;
    struct termios new_tio;
    int current_row;
    int current_col;
} VFD_Display;

// Function declarations
VFD_Display* vfd_init(const char* port);
void vfd_close(VFD_Display* vfd);
void vfd_clear(VFD_Display* vfd);
void vfd_write(VFD_Display* vfd, const char* text);
char* vfd_read(VFD_Display* vfd);

#endif
