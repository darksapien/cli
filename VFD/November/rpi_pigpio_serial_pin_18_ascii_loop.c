#include <stdio.h>
#include <pigpio.h>
#include <string.h>
#include <unistd.h>

#define SERIAL_PIN 18
#define BAUD_RATE 1200
#define DATA_BITS 8
#define STOP_BITS 2  // 1 full stop bit = 2 half bits
#define ASCII_START 32 // Start from space character
#define ASCII_END 126  // End at tilde character

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    // Configure GPIO pin for output
    gpioSetMode(SERIAL_PIN, PI_OUTPUT);

    while (1) { // Infinite loop
        for (int i = ASCII_START; i <= ASCII_END; i++) {
            char data[2];
            data[0] = (char)i; // Current ASCII character
            data[1] = '\0';    // Null-terminate the string

            gpioWaveAddNew();
            gpioWaveAddSerial(SERIAL_PIN, BAUD_RATE, DATA_BITS, STOP_BITS, 0, 1, data);

            int wave_id = gpioWaveCreate();
            if (wave_id >= 0) {
                gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);

                // Wait for the transmission to complete
                while (gpioWaveTxBusy()) {
                    usleep(100);
                }

                gpioWaveDelete(wave_id);
            } else {
                fprintf(stderr, "Failed to create waveform for character %c\n", data[0]);
            }

            usleep(100000); // Delay of 100 ms between characters
        }
    }

    // Cleanup (unreachable in this infinite loop)
    gpioTerminate();
    return 0;
}
