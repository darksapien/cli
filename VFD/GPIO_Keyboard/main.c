//===============================
// FILE: main.c
// compiles with gcc -o vfd_test main.c vfd_gpio.c vfd_display.c -lws2811 -lpigpio -lm -pthread -Wall -I/usr/local/include -L/usr/local/lib
//===============================
#include "vfd_display.h"
#include "vfd_keyboard.h"
#include "vfd_gpio.h"
#include <errno.h>
#include <signal.h>

VFD_Display* vfd = NULL;
GPIO_Control* gpio = NULL;

void cleanup(int signum) {
    if (vfd) vfd_close(vfd);
    if (gpio) gpio_cleanup(gpio);
    keyboard_restore();
    exit(signum);
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

    // Initialize GPIO and NeoPixel
    gpio = gpio_init();
    if (!gpio) {
        fprintf(stderr, "Failed to initialize GPIO control\n");
        cleanup(1);
        return 1;
    }

    // Clear display and prepare keyboard
    vfd_clear(vfd);
    keyboard_raw_mode();

    // Show initial mode
    show_mode_instructions(vfd, gpio->current_mode);
    update_neopixel_mode(gpio);

    // Main loop
    while (1) {
        // Check for mode changes
        if (gpio->mode_changed) {
            gpio->mode_changed = 0;
            show_mode_instructions(vfd, gpio->current_mode);
            continue;
        }

        // Handle input based on current mode
        switch (gpio->current_mode) {
            case MODE_SSH:
            case MODE_LOCAL_KB: {
                int c = read_key();
                if (c < 0) continue;

                // Handle special keys
                switch (c) {
                    case 1001:  // Up arrow
                        write(vfd->fd, VFD_CURSOR_UP, 2);
                        break;

                    case 1002:  // Down arrow
                        write(vfd->fd, VFD_NEWLINE, 2);
                        break;

                    case 1003:  // Right arrow
                        write(vfd->fd, "\x09", 1);
                        break;

                    case 1004:  // Left arrow
                        write(vfd->fd, "\x08", 1);
                        break;

                    case 1005:  // Delete
                        write(vfd->fd, "\x19\x10", 2);
                        break;

                    case 127:   // Backspace
                    case 8:     // Also handle Ctrl-H backspace
                        write(vfd->fd, "\x08", 1);
                        write(vfd->fd, "\x19\x10", 2);
                        break;

                    case 13:    // Enter
                        write(vfd->fd, VFD_NEWLINE, 2);
                        break;

                    default:
                        // Regular printable character
                        if (c >= 32 && c <= 126) {
                            write(vfd->fd, &c, 1);
                        }
                        break;
                }
                break;
            }

            case MODE_AUTO:
                // In auto mode, could read from a file or message queue
                // For now, just show a simple message
                vfd_write(vfd, "Auto mode active... \n");
                sleep(2);
                break;
        }
    }

    cleanup(0);
    return 0;
}