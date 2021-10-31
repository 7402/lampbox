//
//  play.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "play.h"
#include "sound.h"
#include "mute.h"

int16_t *file_data = NULL;

const int MUTE_WAIT_TIME = 100;     // 100 msec

void close_play(void)
{
    if (file_data != NULL) {
        wait_for_buffers();
        free(file_data);
        file_data = NULL;
    }
}

const char *path_for_sound(SoundID sound_ID);
const char *path_for_sound(SoundID sound_ID)
{
    char *path = NULL;
        
    switch (sound_ID) {
        case SOUND_THX:
            path = "/home/pi/Music/THX.wav";
            break;
        case SOUND_CLANK:
            path = "/home/pi/Music/Clank.wav";
            break;
        case SOUND_CLUNK:
            path = "/home/pi/Music/Clunk.wav";
            break;
        case SOUND_BELL:
            path = "/home/pi/Music/Bell-Fight-44M.wav";
            break;
        case SOUND_BEBEK:
            path = "/home/pi/Music/BeBek.wav";
            break;
        case SOUND_EWF:
            path = "/home/pi/Music/LittleGirl-Mono.wav";
            break;
        case SOUND_HIGH_BEEP:
            path = "/home/pi/Music/high_beep.wav";
            break;
        case SOUND_LOW_BEEP:
            path = "/home/pi/Music/low_beep.wav";
            break;
        case SOUND_GOOSE:
            path = "/home/pi/Music/Goose-Canada-Lp-44M.wav";
            break;
        case SOUND_LAUGH:
            path = "/home/pi/Music/F-Laughing-44M.wav";
            break;
        case SOUND_GROWL:
            path = "/home/pi/Music/Wolf-Growl-A-44M.wav";
            break;
        case SOUND_MESSAGE:
            path = "/home/pi/Messages/message.wav";
            break;
        default:
            break;
    }

    return path;
}

void play_sound_async(SoundID sound_ID)
{
    if (file_data != NULL) {
        wait_for_buffers();
        free(file_data);
        file_data = NULL;
    }
    
    const char *path = path_for_sound(sound_ID);

    if (path != NULL) {
        WaveHeader header;
        long file_size;
        
        SoundError error = read_wav(path, &header, &file_data, &file_size);
        
        if (error == SE_NO_ERROR) {
            mute_mic();
            unmute_speaker();
            error = play_wav_data(&header, file_data, file_size);
        }
        
        if (error != SE_NO_ERROR) fprintf(stderr, "play_sound_async() = %s\n", sound_error_text(error));
    }
}

bool mute_if_done_playing_async(void)
{
    bool done = false;
    
    if (!sound_playing() && file_data != NULL) {
        fprintf(stderr, "done playing\n");
        mute_speaker();
        sleep_msec(MUTE_WAIT_TIME);
        unmute_mic();
        done = true;

        free(file_data);
        file_data = NULL;
    }
    
    return done;
}

void play_sound(SoundID sound_ID)
{
    if (file_data != NULL) {
        wait_for_buffers();
        free(file_data);
        file_data = NULL;
    }
    
    const char *path = path_for_sound(sound_ID);

    if (path != NULL) {
        mute_mic();
        unmute_speaker();
        play_wav(path);
        mute_speaker();
        sleep_msec(MUTE_WAIT_TIME);
        unmute_mic();
        
    } else {
        fprintf(stderr, "bad path for play_sound()\n");
    }
}

void play_file(const char *path)
{
    if (file_data != NULL) {
        wait_for_buffers();
        free(file_data);
        file_data = NULL;
    }
    
    if (path != NULL) {
        mute_mic();
        unmute_speaker();
        play_wav(path);
        mute_speaker();
        sleep_msec(MUTE_WAIT_TIME);
        unmute_mic();
    }
}

void sleep_msec(unsigned long msec)
{
    struct timespec ts;
    
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    
    nanosleep(&ts, NULL);
}
