//===============================
// FILE: vfd_display.c
//===============================
#include "vfd_display.h"

VFD_Display* vfd_init(const char* port) {
    VFD_Display* vfd = malloc(sizeof(VFD_Display));
    if (!vfd) return NULL;

    // Open port with specific flags
    vfd->fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (vfd->fd < 0) {
        free(vfd);
        return NULL;
    }

    // Get current settings
    tcgetattr(vfd->fd, &vfd->old_tio);
    memset(&vfd->new_tio, 0, sizeof(struct termios));

    // Configure port settings (7E1)
    // Clear all flags first
    vfd->new_tio.c_cflag = 0;
    vfd->new_tio.c_iflag = 0;
    vfd->new_tio.c_oflag = 0;
    vfd->new_tio.c_lflag = 0;

    // Set the exact flags we need
    vfd->new_tio.c_cflag = CS7 | PARENB | CREAD | CLOCAL;
    vfd->new_tio.c_cc[VMIN] = 0;  // Non-blocking read
    vfd->new_tio.c_cc[VTIME] = 10; // 1 second timeout

    // Set baud rate
    cfsetispeed(&vfd->new_tio, B9600);
    cfsetospeed(&vfd->new_tio, B9600);

    // Flush port and apply settings
    tcflush(vfd->fd, TCIOFLUSH);
    tcsetattr(vfd->fd, TCSANOW, &vfd->new_tio);

    // Enable RTS explicitly
    int flags;
    if (ioctl(vfd->fd, TIOCMGET, &flags) != -1) {
        flags |= TIOCM_RTS;
        ioctl(vfd->fd, TIOCMSET, &flags);
    }

    // Wait for display to stabilize
    usleep(100000);  // 100ms delay

    vfd->current_row = 0;
    vfd->current_col = 0;

    return vfd;
}

void vfd_close(VFD_Display* vfd) {
    if (!vfd) return;
    tcsetattr(vfd->fd, TCSANOW, &vfd->old_tio);
    close(vfd->fd);
    free(vfd);
}

void vfd_clear(VFD_Display* vfd) {
    if (!vfd) return;
    write(vfd->fd, VFD_CLEAR, 2);
    usleep(50000);  // 50ms delay
    vfd->current_row = 0;
    vfd->current_col = 0;
}

void vfd_write(VFD_Display* vfd, const char* text) {
    if (!vfd || !text) return;

    size_t len = strlen(text);
    size_t pos = 0;

    while (pos < len) {
        if (text[pos] == '\n') {
            // Handle newline with CR+LF
            write(vfd->fd, "\x0D\x0A", 2);
            vfd->current_col = 0;
            vfd->current_row++;
            if (vfd->current_row >= VFD_ROWS) {
                vfd->current_row = VFD_ROWS - 1;
            }
        } else {
            // Handle regular character
            write(vfd->fd, &text[pos], 1);
            vfd->current_col++;
            if (vfd->current_col >= VFD_COLS) {
                write(vfd->fd, "\x0D\x0A", 2);
                vfd->current_col = 0;
                vfd->current_row++;
                if (vfd->current_row >= VFD_ROWS) {
                    vfd->current_row = VFD_ROWS - 1;
                }
            }
        }
        pos++;
    }
}

char* vfd_read(VFD_Display* vfd) {
    if (!vfd) return NULL;

    // Send query command
    write(vfd->fd, VFD_QUERY, 2);
    usleep(50000);  // Wait 50ms for response

    // Allocate buffer for response
    // Maximum possible size plus extra room for any control characters
    char* buffer = malloc(VFD_ROWS * VFD_COLS + 10);
    if (!buffer) return NULL;

    // Read response with timeout
    int total_read = 0;
    int attempts = 0;
    const int max_attempts = 10;

    // Keep reading until we timeout
    while (attempts < max_attempts) {
        int bytes = read(vfd->fd, buffer + total_read,
                        (VFD_ROWS * VFD_COLS + 10) - total_read);
        if (bytes > 0) {
            total_read += bytes;
            attempts = 0;  // Reset attempts if we got data
        } else {
            usleep(10000);  // Wait 10ms before next attempt
            attempts++;
        }
    }

    if (total_read == 0) {
        free(buffer);
        return NULL;
    }

    // Add null terminator after whatever we received
    buffer[total_read] = '\0';

    return buffer;
}
