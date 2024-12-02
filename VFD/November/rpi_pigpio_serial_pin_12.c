#include <stdio.h>
#include <pigpio.h>
#include <string.h>
#include <unistd.h>

#define SERIAL_PIN 12
#define BAUD_RATE 1200
#define DATA_BITS 8
#define STOP_BITS 2  // 1 full stop bit = 2 half bits

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    // Configure GPIO pin for output
    gpioSetMode(SERIAL_PIN, PI_OUTPUT);

    // Create a waveform representing the serial data
    char *data = "Hello, Display!";
    gpioWaveAddNew();
    gpioWaveAddSerial(SERIAL_PIN, BAUD_RATE, DATA_BITS, STOP_BITS, 0, strlen(data), data);

    // Send the waveform
    int wave_id = gpioWaveCreate();
    if (wave_id >= 0) {
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
    } else {
        fprintf(stderr, "Failed to create waveform\n");
        gpioTerminate();
        return 1;
    }

    // Wait for the transmission to complete
    while (gpioWaveTxBusy()) {
        usleep(100);
    }

    // Clean up
    gpioWaveDelete(wave_id);
    gpioTerminate();
    return 0;
}
