//===============================
// FILE: vfd_keyboard.h
//===============================
#ifndef VFD_KEYBOARD_H
#define VFD_KEYBOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Terminal settings
struct termios orig_termios;
int is_local_tty = 0;

// Check if we're on a local terminal
int check_local_terminal() {
    char *tty_name = ttyname(STDIN_FILENO);
    if (tty_name && strncmp(tty_name, "/dev/tty", 8) == 0) {
        return 1;  // Local terminal
    }
    return 0;  // SSH or other connection
}

// Initialize raw mode for keyboard input
void keyboard_raw_mode() {
    is_local_tty = check_local_terminal();

    // If local terminal, we need to handle keyboard differently
    if (is_local_tty) {
        system("chvt 1");  // Switch to tty1 if not already there
        system("clear");   // Clear the screen
    }

    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ECHO | ICANON);

     // Allow Ctrl+C but disable other special character handling
    raw.c_lflag &= ~(IEXTEN | ICANON | ECHO);
    raw.c_lflag |= ISIG;  // Make sure ISIG is enabled

    // Disable flow control and other input processing
    raw.c_iflag &= ~(IXON | ICRNL);

    // Set character timeout
    raw.c_cc[VMIN] = 0;   // No minimum characters
    raw.c_cc[VTIME] = 1;  // 0.1 second timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restore original terminal settings
void keyboard_restore() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    if (is_local_tty) {
        printf("\033[?25h");  // Show cursor
        system("clear");      // Clear screen
    }
}

// Read a key, handling escape sequences
int read_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c == 27) {  // ESC character
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 27;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 1001;  // Up arrow
                case 'B': return 1002;  // Down arrow
                case 'C': return 1003;  // Right arrow
                case 'D': return 1004;  // Left arrow
                case '3':  // Delete key
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return 27;
                    if (seq[2] == '~') return 1005;
            }
        }
    }

    return c;
}

#endif
