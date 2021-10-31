//
//  phrases.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "phrases.h"

const char *phrase_list[] = {
    "computer",
    "yes",
    "no",
    "down",
    "blinky",
    "red light",
    "red_light",
    "green light",
    "green_light",
    "blue light",
    "blue_light",
    "yellow light",
    "yellow_light",
    "orange light",
    "orange_light",
    "purple light",
    "purple_light",
    "maroon light",
    "maroon_light",
    "lamp off",
    "lamp_off",
    "okay",
    "repeat after me",
    "repeat_after_me",
    "change voice",
    "alexa",
    "hey siri",
    "hey_siri",
    "hey google",
    "hey_google",
    "exit program",
    "shut down computer",
    "show distance",
    "cancel distance",
    "ready",
    "play music",
    "play_music",
    "play song",
    "play_song",
    "air quality",
    "air_quality",
    "show air quality",
    "show air_quality",
    "weather report",
    "weather_report",
    "think",
    "think harder",
    "sparkle",
    "*",
    ">",
    "ro",
    "rw",
    "record message",
    "play message",
    "play_message",
    "play messages",
    "play_messages",
    "play again",
    "play_again",
    "save",
    "delete",
    "stop",
    "count messages",
    "hello",
    "stop listening",
    "sleep",
    "keep message",
    "keep_message",
    "switch alpha",
    "switch bravo",
    "switch charlie",
    "switch delta",
    "switch echo",
    "switch foxtrot",
    "status alpha",
    "status bravo",
    "status charlie",
    "status delta",
    "status echo",
    "status foxtrot",
    "volume up",
    "volume_up",
    "volume down",
    "volume_down",
    "reset volume"
};
const size_t phrase_count = sizeof(phrase_list) / sizeof(const char *);

PhraseID phrase_to_index(const char *phrase)
{
    assert(phrase_count == END_OF_LIST);
        
    for (size_t k = 0; k < phrase_count; k++) {
        if (strcasecmp(phrase, phrase_list[k]) == 0) return (PhraseID)k;
    }
    
    return UNKNOWN;
}

const char * index_to_phrase(PhraseID index)
{
    if (index >= 0 && index < phrase_count) {
        return phrase_list[index];
        
    } else {
        return "";
    }
}
