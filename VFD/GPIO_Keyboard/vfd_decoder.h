//===============================
// FILE: vfd_decoder.h
//===============================
#ifndef VFD_DECODER_H
#define VFD_DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Convert VFD response to human readable format with ASCII names
char* vfd_decode_response(const char* response) {
    if (!response) return NULL;

    // Allocate buffer - each byte could become up to 5 chars (#XXX)
    // plus room for null terminator
    size_t max_len = strlen(response) * 5 + 1;
    char* decoded = malloc(max_len);
    if (!decoded) return NULL;

    size_t pos = 0;

    for (int i = 0; response[i] != '\0'; i++) {
        unsigned char c = (unsigned char)response[i];

        switch(c) {
            case 0x02:
                pos += sprintf(decoded + pos, "#STX");
                break;
            case 0x03:
                pos += sprintf(decoded + pos, "#ETX");
                break;
            case 0x04:
                pos += sprintf(decoded + pos, "#EOT");
                break;
            case 0x00:
                pos += sprintf(decoded + pos, "#NUL");
                break;
            case 0x07:
                pos += sprintf(decoded + pos, "#BEL");
                break;
            case 0x08:
                pos += sprintf(decoded + pos, "#BS");
                break;
            case 0x09:
                pos += sprintf(decoded + pos, "#HT");
                break;
            case 0x0A:
                pos += sprintf(decoded + pos, "#LF");
                break;
            case 0x0B:
                pos += sprintf(decoded + pos, "#VT");
                break;
            case 0x0C:
                pos += sprintf(decoded + pos, "#FF");
                break;
            case 0x0D:
                pos += sprintf(decoded + pos, "#CR");
                break;
            case 0x0E:
                pos += sprintf(decoded + pos, "#SO");
                break;
            case 0x0F:
                pos += sprintf(decoded + pos, "#SI");
                break;
            case 0x10:
                pos += sprintf(decoded + pos, "#DLE");
                break;
            case 0x19:
                pos += sprintf(decoded + pos, "#EM");
                break;
            default:
                // If it's a printable character, just copy it
                if (c >= 0x20 && c <= 0x7E) {
                    decoded[pos++] = c;
                } else {
                    // Otherwise show hex
                    pos += sprintf(decoded + pos,pos += sprintf(decoded + pos, "#%02X", c);
                }
        }
    }

    decoded[pos] = '\0';
    return decoded;
}

#endif
