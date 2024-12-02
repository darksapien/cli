//===============================
// FILE: vfd_keypad.c
//===============================
#include "vfd_keypad.h"
#include <stdlib.h>

// Initialize the keypad control structure and I2C connection
Keypad_Control* keypad_init(void) {
    printf("Keypad init: Starting initialization...\n");
    
    // Allocate memory for the keypad control structure
    Keypad_Control* keypad = malloc(sizeof(Keypad_Control));
    if (!keypad) {
        printf("Keypad init: Failed to allocate memory\n");
        return NULL;
    }

    printf("Keypad init: Opening I2C device at %s...\n", I2C_DEVICE);
    // Open I2C device
    keypad->file = open(I2C_DEVICE, O_RDWR);
    if (keypad->file < 0) {
        perror("Keypad init: Failed to open the I2C bus");
        free(keypad);
        return NULL;
    }

    printf("Keypad init: Setting I2C slave address to 0x%X...\n", I2C_ADDR);
    // Set up I2C communication with the keypad
    if (ioctl(keypad->file, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Keypad init: Failed to acquire bus access and/or talk to slave");
        close(keypad->file);
        free(keypad);
        return NULL;
    }

    // Initialize state variables
    keypad->key_pressed = 0;
    keypad->last_key = 0;

    printf("Keypad init: Initialization complete\n");
    return keypad;
}

// Clean up keypad resources
void keypad_cleanup(Keypad_Control* keypad) {
    printf("Keypad cleanup: Starting...\n");
    if (!keypad) {
        printf("Keypad cleanup: Keypad is NULL\n");
        return;
    }
    close(keypad->file);
    free(keypad);
    printf("Keypad cleanup: Complete\n");
}

// Read the current state of the keypad
void read_keypad(Keypad_Control* keypad) {
    static int scan_count = 0;
    scan_count++;
    bool debug_output = (scan_count % 100 == 0); // Print debug every 100 scans

    if (!keypad) {
        if (debug_output) printf("Read keypad: Keypad is NULL!\n");
        return;
    }

    if (debug_output) {
        printf("\n=== Keypad scan #%d starting ===\n", scan_count);
    }

    // Row selection values for scanning the keypad matrix
    char row_select[4] = {0xEF, 0xDF, 0xBF, 0x7F}; // Row selection signals
    char buffer[1] = {0}; // Buffer to store read data

    // Scan each row of the keypad
    for (int i = 0; i < 4; ++i) {
        if (debug_output) {
            printf("Writing row select 0x%02X...\n", row_select[i] & 0xFF);
        }

        // Write row selection signal
        if (write(keypad->file, &row_select[i], 1) != 1) {
            if (debug_output) {
                perror("Failed to write to I2C device");
            }
            continue;
        }

        // Wait for keypad to settle
        usleep(10000);

        // Read the column values for the selected row
        if (read(keypad->file, buffer, 1) != 1) {
            if (debug_output) {
                perror("Failed to read from I2C device");
            }
            continue;
        }

        if (debug_output) {
            printf("Row %d Read value: 0x%02X\n", i, buffer[0] & 0xFF);
        }

        // Process the read value based on the keypad matrix
        char key = 0;
        switch (buffer[0]) {
            case 0xEE: key = '1'; break;
            case 0xED: key = '5'; break;
            case 0xEB: key = '9'; break;
            case 0xE7: key = 'A'; break; // F3
            case 0xDE: key = '2'; break;
            case 0xDD: key = '6'; break;
            case 0xDB: key = '0'; break;
            case 0xD7: key = 'B'; break; // F4
            case 0xBE: key = '3'; break;
            case 0xBD: key = '7'; break;
            case 0xBB: key = '*'; break;
            case 0xB7: key = 'C'; break; // F1
            case 0x7E: key = '4'; break;
            case 0x7D: key = '8'; break;
            case 0x7B: key = '#'; break;
            case 0x77: key = 'D'; break; // F2
        }

        // If we got a key value, print it regardless of debug state
        if (key != 0) {
            printf("Key detected! Raw: 0x%02X, Key: %c\n", buffer[0] & 0xFF, key);
        }

        // If a new key is pressed, update the state
        if (key != 0 && key != keypad->last_key) {
            printf("New key press detected! Previous: %c, New: %c\n", 
                   keypad->last_key ? keypad->last_key : ' ', key);
            keypad->last_key = key;
            keypad->key_pressed = 1;
            break;
        }
    }

    if (debug_output) {
        printf("=== Keypad scan complete ===\n\n");
    }
}

// Get the readable name for a key
const char* get_key_name(char key) {
    static char name[20];
    switch (key) {
        case 'A': return "F3";
        case 'B': return "F4";
        case 'C': return "F1";
        case 'D': return "F2";
        case '*': return "C*";
        case '#': return "F#";
        default:
            snprintf(name, sizeof(name), "%c", key);
            return name;
    }
}