#include "vfd_mode_switch.h"
#include <unistd.h>

Mode_Control* mode_init(void) {
    Mode_Control* ctrl = malloc(sizeof(Mode_Control));
    if (!ctrl) return NULL;

    ctrl->current_mode = MODE_SSH;
    ctrl->mode_changed = 0;
    return ctrl;
}

void mode_cleanup(Mode_Control* ctrl) {
    if (ctrl) free(ctrl);
}

const char* get_mode_name(VFD_Mode mode) {
    switch (mode) {
        case MODE_SSH:
            return "SSH Keyboard Input";
        case MODE_LOCAL_KB:
            return "Local Keyboard Input";
        case MODE_AUTO:
            return "Automatic Display";
        default:
            return "Unknown Mode";
    }
}

void show_mode_instructions(VFD_Display* vfd, VFD_Mode mode) {
    if (!vfd) return;
    
    vfd_clear(vfd);
    
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Mode: %s\n", get_mode_name(mode));
    vfd_write(vfd, buffer);
    
    vfd_write(vfd, "Press Ctrl+M to cycle modes\n");
    
    switch (mode) {
        case MODE_SSH:
            vfd_write(vfd, "SSH keyboard input active\n");
            vfd_write(vfd, "Use terminal to type\n");
            break;
            
        case MODE_LOCAL_KB:
            vfd_write(vfd, "Local keyboard input active\n");
            vfd_write(vfd, "Connect USB keyboard to use\n");
            break;
            
        case MODE_AUTO:
            vfd_write(vfd, "Auto display mode active\n");
            vfd_write(vfd, "Messages will display automatically\n");
            break;
    }
    
    usleep(2000000); // 2 second delay
    vfd_clear(vfd);
}
