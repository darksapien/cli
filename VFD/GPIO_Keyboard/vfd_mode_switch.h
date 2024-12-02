#ifndef VFD_MODE_SWITCH_H
#define VFD_MODE_SWITCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfd_display.h"

typedef enum {
    MODE_SSH = 0,
    MODE_LOCAL_KB = 1,
    MODE_AUTO = 2,
    MODE_COUNT
} VFD_Mode;

typedef struct {
    VFD_Mode current_mode;
    volatile int mode_changed;
} Mode_Control;

Mode_Control* mode_init(void);
void mode_cleanup(Mode_Control* ctrl);
const char* get_mode_name(VFD_Mode mode);
void show_mode_instructions(VFD_Display* vfd, VFD_Mode mode);

#endif