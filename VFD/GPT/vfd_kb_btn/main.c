#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "vfd_display.h"
#include "rotary_switch.h"
#include "neopixel_control.h"

#define NEOPIXEL_COUNT 12
#define NEOPIXEL_PIN 18

VFD_Display* vfd = NULL;

void cleanup(int signum) {
    if (vfd) vfd_close(vfd);
    cleanup_neopixel();
    gpioTerminate();
    exit(signum);
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize pigpio\n");
        return 1;
    }

    if (spiOpen(0, 1000000, 0) < 0) {
        fprintf(stderr, "Failed to open SPI channel\n");
        gpioTerminate();
        return 1;
    }
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "vfd_display.h"
#include "rotary_switch.h"
#include "neopixel_control.h"

#define NEOPIXEL_COUNT 12
#define NEOPIXEL_PIN 18

VFD_Display* vfd = NULL;

void cleanup(int signum) {
    if (vfd) vfd_close(vfd);
    cleanup_neopixel();
    gpioTerminate();
    exit(signum);
}

int main() {
    signal(SIGINT, cleanup);Dude the original log has the compile which included this library. . . still getting errors
    signal(SIGTERM, cleanup);

    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize pigpio\n");
        return 1;
    }

    if (spiOpen(0, 1000000, 0) < 0) {
        fprintf(stderr, "Failed to open SPI channel\n");
        gpioTerminate();
        return 1;
    }

    init_neopixel(NEOPIXEL_COUNT, NEOPIXEL_PIN);

    vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD: %s\n", strerror(errno));
        spiClose(0);
        cleanup_neopixel();
        gpioTerminate();
        return 1;
    }

    while (1) {
        int rotary_value = read_rotary_value(1);  // Read rotary switch on channel 1
        int position = get_rotary_position(rotary_value);

        update_neopixel(position);

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Rotary Pos: %d", position);
        vfd_write(vfd, buffer);

        usleep(1000000); // 1-second delay
    }

    cleanup(0);
    return 0;
}

    init_neopixel(NEOPIXEL_COUNT, NEOPIXEL_PIN);

    vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD: %s\n", strerror(errno));
        spiClose(0);
        cleanup_neopixel();
        gpioTerminate();
        return 1;
    }

    while (1) {
        int rotary_value = read_rotary_value(1);  // Read rotary switch on channel 1
        int position = get_rotary_position(rotary_value);

        update_neopixel(position);

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Rotary Pos: %d", position);
        vfd_write(vfd, buffer);

        usleep(1000000); // 1-second delay
    }

    cleanup(0);
    return 0;
}

