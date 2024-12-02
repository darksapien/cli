#include "vfd_display.h"
#include "vfd_keyboard.h"
#include "vfd_mode_switch.h"
#include <errno.h>
#include <signal.h>
#include <ctype.h>

VFD_Display* vfd = NULL;
Mode_Control* mode_ctrl = NULL;

void cleanup(int signum) {
    if (vfd) vfd_close(vfd);
    if (mode_ctrl) mode_cleanup(mode_ctrl);
    keyboard_restore();
    exit(signum);
}

// Function to print console menu
void print_console_menu() {
    printf("\nVFD Input Control Menu:\n");
    printf("Commands:\n");
    printf("  ssh  - Switch to SSH keyboard input\n");
    printf("  raw  - Switch to raw keyboard input\n");
    printf("  mode - Show current mode\n");
    printf("  quit - Exit program\n");
    printf("Current mode: %s\n", get_mode_name(mode_ctrl->current_mode));
    printf("\nEnter command> ");
    fflush(stdout);
}

// Function to process console commands
void process_command(const char* cmd) {
    // Convert command to lowercase
    char lcmd[32] = {0};
    int i;
    for(i = 0; cmd[i] && i < 31; i++) {
        lcmd[i] = tolower(cmd[i]);
    }
    lcmd[i] = '\0';

    if (strcmp(lcmd, "ssh") == 0) {
        mode_ctrl->current_mode = MODE_SSH;
        show_mode_instructions(vfd, mode_ctrl->current_mode);
        printf("Switched to SSH keyboard input mode\n");
    }
    else if (strcmp(lcmd, "raw") == 0) {
        mode_ctrl->current_mode = MODE_LOCAL_KB;
        show_mode_instructions(vfd, mode_ctrl->current_mode);
        printf("Switched to raw keyboard input mode\n");
    }
    else if (strcmp(lcmd, "mode") == 0) {
        printf("Current mode: %s\n", get_mode_name(mode_ctrl->current_mode));
    }
    else if (strcmp(lcmd, "quit") == 0 || strcmp(lcmd, "exit") == 0) {
        cleanup(0);
    }
    else {
        printf("Unknown command. Available commands: ssh, raw, mode, quit\n");
    }
}

int main() {
    // Set up signal handlers
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Initialize VFD
    vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD display: %s\n", strerror(errno));
        return 1;
    }

    // Initialize mode control
    mode_ctrl = mode_init();
    if (!mode_ctrl) {
        fprintf(stderr, "Failed to initialize mode control\n");
        cleanup(1);
        return 1;
    }

    // Clear display
    vfd_clear(vfd);
    
    // Show initial mode
    show_mode_instructions(vfd, mode_ctrl->current_mode);

    // Print initial console menu
    print_console_menu();

    // Main command loop
    char cmd[32];
    while (fgets(cmd, sizeof(cmd), stdin)) {
        // Remove newline
        cmd[strcspn(cmd, "\n")] = 0;
        
        // Process the command
        process_command(cmd);
        
        // Show menu again
        print_console_menu();
    }

    cleanup(0);
    return 0;
}