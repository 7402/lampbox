//
//  speak.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <unistd.h>

#include <flite/flite.h>

#include "speak.h"
#include "mute.h"
#include "play.h"

cst_voice *register_cmu_us_kal16(const char *voxdir);
cst_voice *register_cmu_us_slt(const char *voxdir);
void usenglish_init(cst_voice *v);
cst_lexicon *cmulex_init(void);

VoiceID current_voiceID = VOICE_CLB;

cst_voice *voice_slt = NULL;
cst_voice *voice_clb = NULL;
cst_voice *voice_kal16 = NULL;
cst_voice *voice_jmk = NULL;
cst_voice *voice_bdl = NULL;

void init_speak(void)
{
    flite_init();
    flite_add_lang("eng", usenglish_init, cmulex_init);
    flite_add_lang("usenglish", usenglish_init, cmulex_init);

    voice_slt = register_cmu_us_slt(NULL);
    voice_clb = flite_voice_select("/home/pi/Voices/cmu_us_clb.flitevox");
    voice_kal16 = register_cmu_us_kal16(NULL);
    voice_jmk = flite_voice_select("/home/pi/Voices/cmu_us_jmk.flitevox");
    voice_bdl = flite_voice_select("/home/pi/Voices/cmu_us_bdl.flitevox");
}

VoiceID get_current_voiceID(void)
{
    return current_voiceID;
}

void set_current_voiceID(VoiceID voiceID)
{
    if (voiceID >= 0 && voiceID < VOICE_COUNT) current_voiceID = voiceID;
}

void speak_with_voice(const char *text, VoiceID voiceID)
{
    cst_voice *voice = NULL;
    
    switch (voiceID) {
        case VOICE_KAL16:
            voice = voice_kal16;
            break;
            
        case VOICE_BDL:
            voice = voice_bdl;
            break;
            
//        case VOICE_SLT:
//            voice = voice_slt;
//            break;
            
        case VOICE_CLB:
            voice = voice_clb;
            break;
            
        case VOICE_JMK:
            voice = voice_jmk;
            break;
            
        default:
            break;
            
    }
    
    if (voice != NULL) {

#if 0
            mute_mic();
            unmute_speaker();
            flite_text_to_speech(text, voice, "play");
            mute_speaker();
            sleep_msec(70);
            unmute_mic();
#else
            if (strlen(text) > 0 && text[0] != '>') {
                fprintf(stderr, "speak \"%s\"\n", text);
                
                const char *tmp_file = "/home/pi/tmp/speech.wav";
                
                flite_text_to_speech(text, voice, tmp_file);
                play_file(tmp_file);
            }
#endif
        
    }
}

