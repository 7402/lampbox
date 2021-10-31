//
//  arduino.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "arduino.h"

int serial_fd = -1;

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];
size_t next = 0;
size_t buffer_count;

void init_arduino(void)
{
    serial_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    
    sleep(1);   // https://stackoverflow.com/questions/13013387/clearing-the-serial-ports-buffer

    if (serial_fd > 0) {
        fcntl(serial_fd, F_SETFL, FNDELAY);

        struct termios options;
        tcgetattr(serial_fd, &options);

        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        
        options.c_cflag |= (CLOCAL | CREAD);

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        
        options.c_iflag &= ~(IXON | IXOFF | IXANY);

        options.c_oflag &= ~OPOST;

        tcsetattr(serial_fd, TCSAFLUSH, &options);
    }
}

void close_arduino(void)
{
    sleep(1);
    
    if (serial_fd > 0) {
        close(serial_fd);
        serial_fd = -1;
    }
}

MessageID get_message(void)
{
    MessageID message_ID = NO_MESSAGE;
    bool available = false;
    
    do {
        char c = '\0';
        
        if (buffer_count == 0) {
            fcntl(serial_fd, F_SETFL, FNDELAY);
            ssize_t num_read = read(serial_fd, buffer, BUFFER_SIZE);
            if (num_read > 0) {
                buffer_count = num_read;
            }
        }

        available = next < buffer_count;
        if (available) {
            c = buffer[next++];
            available = true;
            if (next == buffer_count) {
                next = 0;
                buffer_count = 0;
            }
        }

        switch (c) {
            case 'U': message_ID = BUTTON_UP;       break;
            case 'D': message_ID = BUTTON_DOWN;     break;
            case 'E': message_ID = BLINK_END;       break;
            default:                                break;
        }

    } while ((message_ID == NO_MESSAGE) && available);

    return message_ID;
}

void put_message(const char *text)
{
    // fprintf(stderr, "%s\n", text);
    
    ssize_t n = write(serial_fd, text, strlen(text));
    if (n != strlen(text)) {
        fprintf(stderr, "Failed serial write %d bytes\n", (int)n);
    }
}

void blink_left_right(int count)
{
    put_message("C0 H0 L1 L2 L3 L4 L5 T75 L0 H1 T75 L1 H2 T75 L2 H3 T75 L3 H4 T75 L4 H5 T75 ");
    
    if (count == 1) {
        put_message("L5 H4 T75 L4 H3 T75 L3 H2 T75 L2 H1 T75 L1 H0 T75 L0 T75 P1 ");

    } else {
        if (count > 1) count--;

        char str[256];
        sprintf(str, "L5 H4 T75 L4 H3 T75 L3 H2 T75 L2 H1 T75 P%d ", count);
        put_message(str);
    }
}

void blink_think(int count)
{
    if (count == 0) {
        put_message("X0 C0 Q0 Q1 Q2 Q3 Q4 Q5 T200 P0 ");

    } else {
        char str[256];
        sprintf(str, "X0 C0 Q0 Q1 Q2 Q3 Q4 Q5 T200 P%d L0 L1 L2 L3 L4 L5 ", count);
        put_message(str);
    }
}

void pulse_color(const char *high_color_string, const char *low_color_string)
{
    put_message("C0 F0 ");
    put_message(low_color_string);
    put_message("F2000 T100 F0 ");
    put_message(high_color_string);
    put_message("F2000 T100 P0 ");
}

void flash_color(const char *color_string)
{
    put_message("C0 ");
    put_message(color_string);
    put_message("T200 R0 G0 B0 T3000 P0 ");
}
