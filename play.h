//
//  play.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef play_h
#define play_h

#include <stdbool.h>

void close_play(void);

enum SoundID {
    SOUND_THX,
    SOUND_CLANK,
    SOUND_CLUNK,
    SOUND_BELL,
    SOUND_BEBEK,
    SOUND_EWF,
    SOUND_HIGH_BEEP,
    SOUND_LOW_BEEP,
    SOUND_GOOSE,
    SOUND_LAUGH,
    SOUND_GROWL,
    SOUND_MESSAGE,
    SOUND_COUNT
};
typedef enum SoundID SoundID;

void play_sound(SoundID sound_ID);
void play_sound_async(SoundID sound_ID);
bool mute_if_done_playing_async(void);
void play_file(const char *path);

void sleep_msec(unsigned long msec);

#endif /* play_h */
