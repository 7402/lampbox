//
//  arduino.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef arduino_h
#define arduino_h

void init_arduino(void);
void close_arduino(void);

enum MessageID {
    NO_MESSAGE = -1,
    BUTTON_DOWN,
    BUTTON_UP,
    BLINK_END
};
typedef enum MessageID MessageID;

MessageID get_message(void);

void put_message(const char *text);

void blink_left_right(int count);

void blink_think(int count);

void pulse_color(const char *high_color_string, const char *low_color_string);

void flash_color(const char *color_string);

#endif /* arduino_h */
