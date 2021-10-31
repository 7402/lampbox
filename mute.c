//
//  mute.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <gpiod.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include "mute.h"
#include "gpio_ctrl.h"

struct gpiod_line *line_SD = NULL;   // 24

snd_mixer_t *alsa_handle = NULL;
snd_mixer_selem_id_t *sid = NULL;
snd_mixer_elem_t* elem = NULL;

long capture_volume = 221;

#define IGNORED 0
#define DEBUG 0

void init_mute(void)
{
    // Open GPIO chip
    init_gpio_ctrl();

    // Open GPIO line
    line_SD = gpiod_chip_get_line(chip, 24);

    // Open line for output
    gpiod_line_request_output(line_SD, "lampbox", 0);
    
    int code;
    
    code = snd_mixer_open(&alsa_handle, IGNORED);
    if (DEBUG) fprintf(stderr, "snd_mixer_open %d\n", code);
    
    code = snd_mixer_attach(alsa_handle, "default");
    if (DEBUG) fprintf(stderr, "snd_mixer_attach %d\n", code);
    
    code = snd_mixer_selem_register(alsa_handle, NULL, NULL);
    if (DEBUG) fprintf(stderr, "snd_mixer_selem_register %d\n", code);
    
    code = snd_mixer_load(alsa_handle);
    if (DEBUG) fprintf(stderr, "snd_mixer_load %d\n", code);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Mic");

    elem = snd_mixer_find_selem(alsa_handle, sid);
    
    if (elem == NULL) {
        fprintf(stderr, "NULL elem\n");
        
    } else {
        long min;
        long max;
        
        code = snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
        if (DEBUG) fprintf(stderr, "snd_mixer_selem_get_capture_volume_range %d\n", code);
        
        code = snd_mixer_selem_get_capture_volume(elem, 0, &capture_volume);
        if (DEBUG) fprintf(stderr, "snd_mixer_selem_get_capture_volume %d\n", code);

        if (DEBUG) fprintf(stderr, "elem value %ld min %ld max %ld\n", capture_volume, min, max);
        if (DEBUG) fprintf(stderr, "elem value %5.1f%%\n",
                           100.0 * (float)(capture_volume - min) / (float)(max - min));
        
        int switch_value;
        code = snd_mixer_selem_get_capture_switch(elem, 0, &switch_value);
        if (DEBUG) fprintf(stderr, "snd_mixer_selem_get_capture_switch %d\n", code);
        
        if (DEBUG) fprintf(stderr, "elem capture_switch %d\n", switch_value);
    }
}

void close_mute(void)
{
    close_gpio_ctrl();
    line_SD = NULL;
    
    snd_mixer_close(alsa_handle);
}

void mute_speaker(void)
{
    gpiod_line_set_value(line_SD, 0);
}

void unmute_speaker(void)
{
    gpiod_line_set_value(line_SD, 1);
}

void mute_mic(void)
{
    if (elem != NULL) {
        int code = snd_mixer_selem_set_capture_switch(elem, 0, 0);
        if (DEBUG) fprintf(stderr, "snd_mixer_selem_set_capture_switch to 0 %d\n", code);
    }
}

void unmute_mic(void)
{
    if (elem != NULL) {
        int code = snd_mixer_selem_set_capture_switch(elem, 0, 1);
        if (DEBUG) fprintf(stderr, "snd_mixer_selem_set_capture_switch to 1 %d\n", code);
    }
}
