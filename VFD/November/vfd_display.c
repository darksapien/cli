#include "vfd_display.h"

VFD_Display* vfd_init(const char* port) {
    VFD_Display* vfd = malloc(sizeof(VFD_Display));
    if (!vfd) return NULL;

    vfd->fd = open(port, O_RDWR | O_NOCTTY);
    if (vfd->fd < 0) {
        free(vfd);
        return NULL;
    }

    // Get current settings
    tcgetattr(vfd->fd, &vfd->old_tio);
    vfd->new_tio = vfd->old_tio;

    // Configure port settings (7E1)
    vfd->new_tio.c_cflag = B9600 | CS7 | PARENB | CLOCAL | CREAD;
    vfd->new_tio.c_iflag = 0;
    vfd->new_tio.c_oflag = 0;
    vfd->new_tio.c_lflag = 0;
    vfd->new_tio.c_cc[VMIN] = 1;
    vfd->new_tio.c_cc[VTIME] = 0;

    // Apply settings
    tcflush(vfd->fd, TCIOFLUSH);
    tcsetattr(vfd->fd, TCSANOW, &vfd->new_tio);

    // Initialize display
    write(vfd->fd, VFD_CLEAR VFD_HOME VFD_BRIGHT VFD_SCROLL, 5);
    
    vfd->current_row = 0;
    vfd->current_col = 0;
    
    return vfd;
}

void vfd_close(VFD_Display* vfd) {
    if (!vfd) return;
    
    // Restore old port settings
    tcsetattr(vfd->fd, TCSANOW, &vfd->old_tio);
    close(vfd->fd);
    free(vfd);
}

void vfd_set_cursor(VFD_Display* vfd, int row, int col) {
    if (!vfd || row < 0 || row >= VFD_ROWS || col < 0 || col >= VFD_COLS) return;
    
    // First position the row (19h 11h followed by row number)
    char row_cmd[3] = {0x19, 0x11, row + 1};  // Row numbers start at 1
    write(vfd->fd, row_cmd, 3);
    
    // Then position the column (19h 13h followed by column number)
    char col_cmd[3] = {0x19, 0x13, col + 1};  // Column numbers start at 1
    write(vfd->fd, col_cmd, 3);
    
    vfd->current_row = row;
    vfd->current_col = col;
}

void vfd_write(VFD_Display* vfd, const char* text) {
    if (!vfd || !text) return;
    
    size_t len = strlen(text);
    size_t pos = 0;
    
    while (pos < len) {
        // Handle newline
        if (text[pos] == '\n') {
            vfd->current_row++;
            vfd->current_col = 0;
            if (vfd->current_row >= VFD_ROWS) {
                // We've reached the bottom, scroll mode will handle it
                write(vfd->fd, VFD_NEWLINE, 2);
                vfd->current_row = VFD_ROWS - 1;
            } else {
                vfd_set_cursor(vfd, vfd->current_row, 0);
            }
            pos++;
            continue;
        }
        
        // Handle normal character
        if (vfd->current_col >= VFD_COLS) {
            // Auto wrap to next line
            vfd->current_row++;
            vfd->current_col = 0;
            if (vfd->current_row >= VFD_ROWS) {
                write(vfd->fd, VFD_NEWLINE, 2);
                vfd->current_row = VFD_ROWS - 1;
            } else {
                vfd_set_cursor(vfd, vfd->current_row, 0);
            }
        }
        
        write(vfd->fd, &text[pos], 1);
        vfd->current_col++;
        pos++;
    }
}

void vfd_clear(VFD_Display* vfd) {
    if (!vfd) return;
    write(vfd->fd, VFD_CLEAR VFD_HOME, 3);
    vfd->current_row = 0;
    vfd->current_col = 0;
}

void vfd_write_blink(VFD_Display* vfd, const char* text) {
    if (!vfd || !text) return;
    write(vfd->fd, VFD_BLINK_START, 1);
    vfd_write(vfd, text);
    write(vfd->fd, VFD_BLINK_END, 1);
}

char* vfd_query_contents(VFD_Display* vfd) {
    if (!vfd) return NULL;
    
    char* buffer = malloc(VFD_ROWS * VFD_COLS + 1); // 240 chars + null terminator
    if (!buffer) return NULL;
    
    // Send query command
    write(vfd->fd, "\x04\x00", 2);
    
    // Read response (blocking)
    int bytes_read = read(vfd->fd, buffer, VFD_ROWS * VFD_COLS);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return buffer;
    }
    
    free(buffer);
    return NULL;
}