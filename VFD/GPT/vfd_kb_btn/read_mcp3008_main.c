#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pigpio.h>
#include "vfd_display.h"
#include "vfd_gpio.h"

#define SPI_CHANNEL 0
#define SPI_SPEED 1000000 // 1 MHz

VFD_Display* vfd = NULL;

// Read from MCP3008
int read_mcp3008(int channel) {
    char tx_buffer[3] = {1, (8 + channel) << 4, 0};
    char rx_buffer[3];
    spiXfer(SPI_CHANNEL, tx_buffer, rx_buffer, 3);
    return ((rx_buffer[1] & 3) << 8) | rx_buffer[2]; // 10-bit result
}

// Cleanup function
void cleanup(int signum) {
    if (vfd) vfd_close(vfd);
    gpioTerminate();
    exit(signum);
}

int main() {
    // Signal handling
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Initialize pigpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize pigpio\n");
        return 1;
    }

    // Initialize SPI
    if (spiOpen(SPI_CHANNEL, SPI_SPEED, 0) < 0) {
        fprintf(stderr, "Failed to open SPI channel\n");
        gpioTerminate();
        return 1;
    }

    // Initialize VFD
    vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD display: %s\n", strerror(errno));
        spiClose(SPI_CHANNEL);
        gpioTerminate();
        return 1;
    }

    char buffer[64];
    while (1) {
        // Display channels 0-3
        snprintf(buffer, sizeof(buffer), "CH0:%d CH1:%d CH2:%d CH3:%d\n",
                 read_mcp3008(0), read_mcp3008(1), read_mcp3008(2), read_mcp3008(3));
        vfd_write(vfd, buffer);

        // Delay for 1 second
        usleep(1000000);
    }

    cleanup(0);
    return 0;
}

