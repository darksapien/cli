#include <stdio.h>
#include "vfd_display.h"

int main() {
    VFD_Display* vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD display\n");
        return 1;
    }

    // Clear display
    vfd_clear(vfd);
    
    // Write a test pattern that shows the 6x40 layout
    vfd_write(vfd, "Line 1 (1234567890) - 40 chars width test....\n");
    vfd_write(vfd, "Line 2: Testing automatic line wrapping......\n");
    vfd_write(vfd, "Line 3: This text will automatically wrap....\n");
    vfd_write(vfd, "Line 4: Display has 6 rows total.............\n");
    vfd_write(vfd, "Line 5: Getting close to the bottom..........\n");
    vfd_write_blink(vfd, "Line 6: This last line will blink!");
    
    // Query display contents
    char* contents = vfd_query_contents(vfd);
    if (contents) {
        printf("Display contents: %s\n", contents);
        free(contents);
    }
    
    vfd_close(vfd);
    return 0;
}
