//
//  speak.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef speak_h
#define speak_h

enum VoiceID {
    VOICE_BDL,
    VOICE_CLB,
    VOICE_JMK,
    VOICE_KAL16,
    VOICE_COUNT
};
typedef enum VoiceID VoiceID;

void init_speak(void);

VoiceID get_current_voiceID(void);

void set_current_voiceID(VoiceID voiceID);

void speak_with_voice(const char *text, VoiceID voiceID);

#endif /* speak_h */
