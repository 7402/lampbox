//
//  main.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "phrases.h"
#include "speak.h"
#include "listen.h"
#include "mute.h"
#include "play.h"
#include "record.h"
#include "sound.h"
#include "arduino.h"
#include "sonar.h"
#include "parse_web.h"

enum RGB_LED_State {
    RGB_OFF,
    AIR_QUALITY,
    COLOR,
    SPARKLE,
    RGB_MESSAGE,
    RGB_RECORDING
};
typedef enum RGB_LED_State RGB_LED_State;

enum BLUE_LEDS_State {
    BLUE_OFF,
    DISTANCE,
    LEFT_RIGHT,
    BLUE_MESSAGE,
    BLINKING,
    BLUE_WOKE
};
typedef enum BLUE_LEDS_State BLUE_LEDS_State;

RGB_LED_State rgb_led_state = RGB_OFF;
BLUE_LEDS_State blue_leds_state = BLUE_OFF;

void all_off(void);
void all_off(void)
{
    put_message("X0 R 0 G 0 B 0 L0 L1 L2 L3 L4 L5 ");
    rgb_led_state = RGB_OFF;
    blue_leds_state = BLUE_OFF;
}
void speak(const char *text);
void speak(const char *text)
{
    blink_think(0);
    speak_with_voice(text, get_current_voiceID());
    all_off();
}


char *rgb_state_text(RGB_LED_State state);
char *rgb_state_text(RGB_LED_State state)
{
    switch (state) {
        case RGB_OFF:       return "RGB_OFF";
        case AIR_QUALITY:   return "AIR_QUALITY";
        case COLOR:         return "COLOR";
        case SPARKLE:       return "SPARKLE";
        case RGB_MESSAGE:   return "RGB_MESSAGE";
        default:            return "unknown";
    }
}

char *blue_state_text(BLUE_LEDS_State state);
char *blue_state_text(BLUE_LEDS_State state)
{
    switch (state) {
        case BLUE_OFF:      return "BLUE_OFF";
        case DISTANCE:      return "DISTANCE";
        case LEFT_RIGHT:    return "LEFT_RIGHT";
        case BLUE_MESSAGE:  return "BLUE_MESSAGE";
        case BLINKING:      return "BLINKING";
        case BLUE_WOKE:     return "BLUE_WOKE";
        default:            return "unknown";
    }
}

void answer_awakening(void);
void answer_awakening(void)
{
    switch (get_current_voiceID()) {
        case VOICE_BDL:
            speak("wuhda-ya want");
            break;
        case VOICE_CLB:
            speak("working");
            break;
        case VOICE_JMK:
            speak("how may I help you");
            break;
        default:
            break;
    }
}

bool file_exists(const char *path);
bool file_exists(const char *path)
{
    FILE *file = fopen(path, "r");
    bool exists = file != NULL;
    if (exists) fclose(file);
    
    return exists;
}

const char *high_pulse_color_for_aqi(double aqi_value);
const char *high_pulse_color_for_aqi(double aqi_value)
{
    if (aqi_value < 0) {
        return "R0 G0 B0 ";    // not available

    } else if (aqi_value <= 50) {
        return "R0 G255 B0 ";     // green

    } else if (aqi_value <= 100) {
        return "R255 G240 B0 ";   // yellow

    } else if (aqi_value <= 150) {
        return "R255 G60 B0 ";    // orange

    } else if (aqi_value <= 200) {
        return "R255 G0 B0 ";     // red

    } else if (aqi_value <= 300) {
        return "R255 G0 B255 ";   // purple

    } else {
        return "R228 G0 B35 ";    // maroon
    }

}

const char *low_pulse_color_for_aqi(double aqi_value);
const char *low_pulse_color_for_aqi(double aqi_value)
{
    if (aqi_value < 0) {
        return "R0 G0 B0 ";       // off

    } else if (aqi_value <= 50) {
        return "R0 G13 B0 ";     // green

    } else if (aqi_value <= 100) {
        return "R13 G12 B0 ";   // yellow

    } else if (aqi_value <= 150) {
        return "R13 G3 B0 ";    // orange

    } else if (aqi_value <= 200) {
        return "R13 G0 B0 ";     // red

    } else if (aqi_value <= 300) {
        return "R13 G0 B13 ";   // purple

    } else {
        return "R12 G0 B2 ";    // maroon
    }

}

const double WAKE_DISTANCE = 50.0;

const char *blue_leds_for_distance(double distance);
const char *blue_leds_for_distance(double distance)
{
    if (distance <= WAKE_DISTANCE) {
        return "L0 L1 H2 H3 L4 L5 ";

    } else if (distance <= 1.6 * WAKE_DISTANCE) {
        return "L0 H1 L2 L3 H4 L5 ";

    } else if (distance <= 2.5 * WAKE_DISTANCE) {
        return "H0 L1 L2 L3 L4 H5 ";

    } else {
        return "L0 L1 L2 L3 L4 L5 ";
    }
}

void wake_blue_leds(void);
void wake_blue_leds(void)
{
    put_message("H0 H1 H2 H3 H4 H5 ");
    sleep_msec(300);
    put_message("L0 L1 L2 L3 L4 L5 ");
    sleep_msec(300);
    put_message("H0 L1 H2 H3 L4 H5 ");
    blue_leds_state = BLUE_WOKE;
}

#define DISTANCE_BUFFER_SIZE 3
double distance_buffer[DISTANCE_BUFFER_SIZE];
int next_distance = 0;

int compare_double(const void *a, const void *b);
int compare_double(const void *a, const void *b)
{
   return *(double *)a - *(double *)b;
}

double median_distance(void);
double median_distance(void)
{
    double temp[DISTANCE_BUFFER_SIZE];
    
    for (int k = 0; k < DISTANCE_BUFFER_SIZE; k++) temp[k] = distance_buffer[k];
    
    qsort(temp, DISTANCE_BUFFER_SIZE, sizeof(double), compare_double);
    
    return temp[ DISTANCE_BUFFER_SIZE / 2 ];
}

const char *message_path = "/home/pi/Messages/message.wav";

int message_count(void);
int message_count(void)
{
     return file_exists(message_path) ? 1 : 0;
}

int blink_and_get_air_quality(void);
int blink_and_get_air_quality(void)
{
    int air_quality = -1;
    
    all_off();
    blink_left_right(0);
    
    int tries = 0;
    do {
        air_quality = get_PM2p5();
        
    } while (air_quality < 0 && ++tries < 3);
    
    all_off();

    return air_quality;
}

bool switch_alpha = false;
bool switch_bravo = false;
bool switch_charlie = false;
bool switch_delta = false;
bool switch_echo = false;
bool switch_foxtrot = false;

const int MIN_SPEAKER_LEVEL = -1;
const int MAX_SPEAKER_LEVEL = 5;
int current_speaker_level = 0;

void set_speaker_level(int level);
void set_speaker_level(int level)
{
    if (level >= MIN_SPEAKER_LEVEL && level <= MAX_SPEAKER_LEVEL) {
        char cmd[256];
        sprintf(cmd, "amixer set Speaker %d%%", 38 + 10 * level);
        system(cmd);
        
        current_speaker_level = level;
    }
}

void say_speaker_level(void);
void say_speaker_level(void)
{
    char txt[256];
    sprintf(txt, "volume level at %d", current_speaker_level);
    speak(txt);
}

void ntp_sync(void);
void ntp_sync(void)
{
    rw();
    system("sudo service ntp stop");
    system("sudo ntpdate 0.us.pool.ntp.org");
    system("sudo service ntp start");
    ro();
}

int main(int argc, const char *argv[]) {
    
    // r/o system may mess up time sync, so force it here
    ntp_sync();
    
    bool use_listen = argc < 2 || strcmp(argv[1], "--nolisten") != 0;
    
    for (int k = 0; k < DISTANCE_BUFFER_SIZE; k++) distance_buffer[k] = 10 * WAKE_DISTANCE;
        
    init_mute();
    mute_speaker();

    init_arduino();
    
    all_off();
    
    if (use_listen) {
        blink_left_right(0);
        blue_leds_state = LEFT_RIGHT;
    }
    
    set_speaker_level(0);

    open_sound();
    if (use_listen) play_sound_async(SOUND_THX);
    
    init_speak();
    init_sonar();
    init_parse_web();

    int input_fd = -1;
    FILE *input = NULL;
    int listen_in_fd = -1;
    FILE *output = NULL;

    if (use_listen) {
        input_fd = init_listen(&listen_in_fd);
        input = fdopen(input_fd, "r");
        output = fdopen(listen_in_fd, "w");

    } else {
        input_fd = STDIN_FILENO;
        input = stdin;
        fprintf(stderr, ">\n");
    }

    char buf[BUFSIZ];
    
    bool done = false;
    bool repeat_after_me = false;
    bool pending_shutdown = false;
    bool pending_exit = false;
    bool listening_paused = false;
    bool recording = false;
    bool was_recording = false;

    bool show_air_quality = false;
    unsigned long long previous_air_quality_index_time = 0;
    const unsigned long long AIR_QUALITY_TIME_INTERVAL = 20LL * 60LL * 1000LL * MSEC;
    int air_quality = -1;
    int previous_air_quality = -1;

    bool woke = false;
    unsigned long long woke_time = 0;
    const unsigned long long STAY_WOKE_TIME = 20LL * 1000LL * MSEC;

    bool show_distance = false;
    unsigned long long previous_distance_time = 0;
    const unsigned long long DISTANCE_TIME_INTERVAL = 300LL * MSEC;
    double distance = -1.0;
    double previous_distance = -1.0;

    unsigned long long previous_hello_time = 0;
    int hello_count = 0;
    const unsigned long long STALE_HELLO_TIME_INTERVAL = 60LL * 1000LL * MSEC;

    unsigned long long previous_ow_time = 0;
    int ow_count = 0;
    const unsigned long long STALE_OW_TIME_INTERVAL = 60LL * 1000LL * MSEC;

    unsigned long long previous_close_enough_time = 0;
    const unsigned long long CLOSE_ENOUGH_LATCH_TIME = 3LL * 1000LL * MSEC;

    bool show_message_alert = message_count() > 0;
    
    if (use_listen) {
        while (!mute_if_done_playing_async()) ;
        all_off();
        rgb_led_state = RGB_OFF;
        blue_leds_state = BLUE_OFF;
    }

    while (!done) {
        if (show_air_quality) {
//            if (air_quality < 0) fprintf(stderr, "time %llud previous %llud\n", get_usec(),
//                                        previous_air_quality_index_time + AIR_QUALITY_TIME_INTERVAL);
            
            if (get_usec() > previous_air_quality_index_time + AIR_QUALITY_TIME_INTERVAL) {
                fprintf(stderr, "getting air quality\n");
//                fprintf(stderr, "air_quality = %d\n", air_quality);
//                fprintf(stderr, "previous_air_quality_index_time = %llud\n", previous_air_quality_index_time);
//                fprintf(stderr, "previous_air_quality_index_time + AIR_QUALITY_TIME_INTERVAL = %llud\n", previous_air_quality_index_time + AIR_QUALITY_TIME_INTERVAL);
//                fprintf(stderr, "AIR_QUALITY_TIME_INTERVAL = %llud\n", AIR_QUALITY_TIME_INTERVAL);
//                fprintf(stderr, "get_usec() = %llud\n", get_usec());
                
                air_quality = blink_and_get_air_quality();
                previous_air_quality_index_time = get_usec();
            }
        }
        
        if (get_usec() > previous_distance_time + DISTANCE_TIME_INTERVAL) {
            double new_distance = get_distance();
            if (new_distance > 0.0) {
                distance_buffer[next_distance] = new_distance;
                next_distance = (next_distance + 1) % DISTANCE_BUFFER_SIZE;

                distance = median_distance();
                // fprintf(stderr, "new_distance = %5.1f median = %5.1f\n", new_distance, distance);
                
                if (distance <= WAKE_DISTANCE) previous_close_enough_time = get_usec();

                previous_distance_time = get_usec();
            }
        }

        if (!woke && get_usec() <= previous_close_enough_time + CLOSE_ENOUGH_LATCH_TIME) {
            distance = WAKE_DISTANCE;
        }

        switch(get_message()) {
            case BUTTON_DOWN:
                fprintf(stderr, "Down\n");
                play_sound(SOUND_CLUNK);
                repeat_after_me = false;
                if (recording) {
                    stop_recording();
                    unmute_speaker();
                    recording = false;
                    was_recording = true;
                    show_message_alert = true;
                    woke_time = get_usec();
                }
                
                if (listening_paused) {
                    listening_paused = false;
                    fprintf(output, "r\n");
                    fflush(output);
                }
                break;
            case BUTTON_UP:
                fprintf(stderr, "Up\n");
                play_sound(SOUND_CLANK);
                
                if (was_recording) {
                    was_recording = false;
                    
                } else if (woke) {
                    woke = false;
                
                } else {
                    if (get_usec() > previous_ow_time + STALE_OW_TIME_INTERVAL) ow_count = 0;
                    
                    sleep_msec(700);
                    if (ow_count < 3) {
                        speak_with_voice("how", VOICE_KAL16);

                    } else if (ow_count == 3) {
                        sleep_msec(1000);
                        speak_with_voice("you are a psychopath", VOICE_KAL16);
                    }
                    
                    ow_count++;
                    previous_ow_time = get_usec();
                }
                break;
            case BLINK_END:
                if (rgb_led_state == SPARKLE || blue_leds_state == BLINKING) {
                    fprintf(stderr, "BLINK_END\n");
                    all_off();
                    rgb_led_state = RGB_OFF;
                    blue_leds_state = BLUE_OFF;
                }
                break;
            default:
                break;
        }
        
        int available = 0;
        ioctl(input_fd, FIONREAD, &available);
        if (available > 0 && fgets(buf, BUFSIZ, input) != NULL) {
            buf[strlen(buf) - 1] = '\0';
            // fprintf(stderr, "listen: '%s'\n", buf);
     
            double confidence;
            int consumed;
            char *buf_p = buf;
            
            if (sscanf(buf, "%lf%n", &confidence, &consumed) != 1) {
                confidence = -1.0;
                
            } else {
                buf_p += consumed;
                if (*buf_p == ' ') buf_p++;
            }
            
            PhraseID cmd = phrase_to_index(buf_p);
            
            if (!woke) {
                if ((distance <= WAKE_DISTANCE || !use_listen) && cmd == COMPUTER_) {
                    woke = true;
                    woke_time = get_usec();
                    answer_awakening();
                    blue_leds_state = BLUE_OFF;
                }
            
            } else if (cmd == ASTERISK) {
                // not recognized
                play_sound(SOUND_LOW_BEEP);

            } else if (cmd == GREATER_THAN || cmd == UNKNOWN) {
                // silence - ignore
                
            } else if (cmd > UNKNOWN && cmd < END_OF_LIST) {
                if (repeat_after_me) {
                    speak(index_to_phrase(cmd));
                    blue_leds_state = BLUE_OFF;

                } else {
                    switch(cmd) {
                        case READY_:
                            fprintf(stderr, "READY\n");
                            break;
                        case RED_LIGHT_:
                        case RED__LIGHT_:
                            put_message("R255 G0 B0 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case GREEN_LIGHT_:
                        case GREEN__LIGHT_:
                            put_message("R0 G255 B0 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case BLUE_LIGHT_:
                        case BLUE__LIGHT_:
                            put_message("R0 G0 B255 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case YELLOW_LIGHT_:
                        case YELLOW__LIGHT_:
                            put_message("R255 G240 B0 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case ORANGE_LIGHT_:
                        case ORANGE__LIGHT_:
                            put_message("R255 G60 B0 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                           break;
                        case PURPLE_LIGHT_:
                        case PURPLE__LIGHT_:
                            put_message("R255 G0 B255 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case MAROON_LIGHT_:
                        case MAROON__LIGHT_:
                            put_message("R228 G0 B35 ");
                            rgb_led_state = COLOR;
                            show_air_quality = false;
                            break;
                        case LAMP_OFF_:
                        case LAMP__OFF_:
                            all_off();
                            show_air_quality = false;
                            show_distance = false;
                            break;
                        case BLINKY_:
                            all_off();
                            blink_left_right(1);
                            blue_leds_state = BLINKING;
                            break;
                        case CHANGE_VOICE_:
                            do {
                                set_current_voiceID((get_current_voiceID() + 1) % VOICE_COUNT);
                            } while (get_current_voiceID() == VOICE_KAL16);
                            
                            answer_awakening();
                            blue_leds_state = BLUE_OFF;
                            break;
                        case COMPUTER_:
                            answer_awakening();
                            blue_leds_state = BLUE_OFF;
                            break;
                        case SLEEP_:
                            speak("okay");
                            woke = false;
                            break;
                        case EXIT_PROGRAM_:
                            pending_exit = true;
                            speak("are you sure about exiting");
                            blue_leds_state = BLUE_OFF;
                            break;
                        case SHUT_DOWN_COMPUTER_:
                            pending_shutdown = true;
                            speak("are you sure about shutting down");
                            blue_leds_state = BLUE_OFF;
                            break;
                        case SHOW_DISTANCE_:
                            show_distance = true;
                            break;
                        case CANCEL_DISTANCE_:
                            show_distance = false;
                            all_off();
                            break;
                        case YES_:
                            if (pending_exit) {
                                speak("good bye");
                                all_off();
                                done = true;
                            };
                            
                            if (pending_shutdown) {
                                speak("good bye");
                                all_off();
                                put_message("C0 R255 G255 B255 T1000 R0 G0 B0 T1000 P0 ");
                                system("sudo poweroff");
                                exit(0);
                            };
                            break;
                        case NO_:
                            pending_exit = false;
                            pending_shutdown = false;
                            break;
                        case REPEAT_AFTER_ME_:
                        case REPEAT__AFTER__ME_:
                            repeat_after_me = true;
                            speak("repeat after me");
                            blue_leds_state = BLUE_OFF;
                            break;
                        case PLAY_MUSIC_:
                        case PLAY__MUSIC_:
                            play_sound(SOUND_EWF);
                            break;
                        case PLAY_SONG_:
                        case PLAY__SONG_:
                            play_sound(SOUND_BEBEK);
                            break;
                        case RECORD_MESSAGE_:
                            recording = true;
                            speak("wait for beep then push button when done recording");
                            blue_leds_state = BLUE_OFF;
                            start_recording(message_path);
                            sleep_msec(500);
                            // hackish: get sound out before arecord has time to launch & start
                            play_sound(SOUND_HIGH_BEEP);
                            mute_speaker();
                            fprintf(stderr, "recording.\n");
                            if (listen_in_fd > 0) {
                                fprintf(output, "p\n");
                                fflush(output);
                                listening_paused = true;
                                fprintf(stderr, "listening paused.\n");
                            }
                            break;
                        case PLAY_MESSAGE_:
                        case PLAY__MESSAGE_:
                        case PLAY_MESSAGES_:
                        case PLAY__MESSAGES_:
                        case PLAY_AGAIN_:
                        case PLAY__AGAIN_:
                            if (message_count() == 0) {
                                play_sound(SOUND_LOW_BEEP);
                                speak("no message");
                                blue_leds_state = BLUE_OFF;

                            } else {
                                play_file(message_path);
                                play_sound(SOUND_HIGH_BEEP);
                                speak("save delete keep message or play again");
                                blue_leds_state = BLUE_OFF;
                            }
                            break;
                        case SAVE_:
                            speak("okay");
                            all_off();
                            show_message_alert = false;
                            break;
                        case KEEP_MESSAGE_:
                        case KEEP__MESSAGE_:
                            speak("okay");
                            all_off();
                            show_message_alert = message_count() > 0;
                            break;
                        case DELETE_:
                            speak("okay");
                            rw();
                            remove(message_path);
                            ro();
                            all_off();
                            show_message_alert = false;
                            break;
                        case STOP_LISTENING_:
                            if (listen_in_fd > 0) {
                                fprintf(output, "p\n");
                                fflush(output);
                                listening_paused = true;
                                fprintf(stderr, "listening paused.\n");
                            }
                            break;
                        case SHOW_AIR_QUALITY_:
                        case SHOW_AIR__QUALITY_:
                            speak("okay");
                            blue_leds_state = BLUE_OFF;
                            show_air_quality = true;
                            air_quality = blink_and_get_air_quality();
                            previous_air_quality_index_time = get_usec();
                            break;
                        case AIR_QUALITY_:
                        case AIR__QUALITY_:
                        {
                            air_quality = blink_and_get_air_quality();
                            previous_air_quality_index_time = get_usec();

                            if (air_quality < 0) {
                                speak("air quality unavailable.");
                                blue_leds_state = BLUE_OFF;
                                
                            } else {
                                char text[32];
                                sprintf(text, "PM 2.5 air quality index is %d", air_quality);
                                speak(text);
                                
                                if (air_quality <= 50) {
                                    speak("good");
                                    
                                } else if (air_quality <= 100) {
                                    speak("moderate");
                                    
                                } else if (air_quality <= 150) {
                                    speak("unhealthy for sensitive groups");
                                    
                                } else if (air_quality <= 200) {
                                    speak("unhealthy");
                                    
                                } else if (air_quality <= 300) {
                                    speak("very unhealthy");
                                    
                                } else {
                                    speak("hazardous");
                                }
                                
                                blue_leds_state = BLUE_OFF;
                            }
                        }
                            break;
                        case WEATHER_REPORT_:
                        case WEATHER__REPORT_:
                        {
                            all_off();
                            blink_left_right(0);
                            WeatherReport *weather = read_weather();
                            all_off();

                            if ((weather->current_condition != NULL &&
                                 strcmp(weather->current_condition, "NA") != 0) || weather->current_temp > -460 ||
                                weather->current_humidity >= 0) {
                                
                                speak("currently");
                                
                                if (weather->current_condition != NULL &&
                                    strcmp(weather->current_condition, "NA") != 0) {
                                    speak(weather->current_condition);
                                }
                                
                                if (weather->current_temp > -460) {
                                    char text[32];
                                    sprintf(text, "%d degrees", (int)round(weather->current_temp));
                                    speak(text);
                                }
                                
                                if (weather->current_humidity >= 0) {
                                    char text[32];
                                    sprintf(text, "%d percent humidity", (int)round(weather->current_humidity));
                                    speak(text);
                                }

                            } else {
                                speak("current weather unavailable.");
                            }
                            
                            if (weather->next_period_name != NULL && weather->next_period_forecast != NULL) {
                                speak(weather->next_period_name);
                                speak(weather->next_period_forecast);
                            }
                            
                            blue_leds_state = BLUE_OFF;
                        }
                            break;
                        case THINK_:
                            blink_think(30);
                            blue_leds_state = BLINKING;
                            break;
                        case THINK_HARDER_:
                            put_message("X0 C0 Q0 Q1 Q2 Q3 Q4 Q5 T50 P120 L0 L1 L2 L3 L4 L5 ");
                            blue_leds_state = BLINKING;
                            break;
                        case SPARKLE_:
                            put_message("X0 C0 S0 T15 P400 R0 G0 B0 ");
                            rgb_led_state = SPARKLE;
                            break;
                        case RO_:
                            ro();
                            break;
                        case RW_:
                            rw();
                            break;
                        case ALEXA_:
                            play_sound(SOUND_LAUGH);
                            break;
                        case HEY_SIRI_:
                        case HEY__SIRI_:
                            play_sound(SOUND_GROWL);
                            break;
                        case HEY_GOOGLE_:
                        case HEY__GOOGLE_:
                            play_sound(SOUND_GOOSE);
                            break;
                        case HELLO_:
                            if (get_usec() > previous_hello_time + STALE_HELLO_TIME_INTERVAL) hello_count = 0;
                            switch (hello_count++) {
                                case 0:
                                    speak("hello");
                                    break;
                                case 1:
                                    speak("hello again");
                                    break;
                                case 2:
                                    speak("hello hello");
                                    break;
                                case 3:
                                    break;
                                case 4:
                                    sleep(1);
                                    speak("boring");
                                    break;
                                default:
                                    break;
                            }
                            
                            blue_leds_state = BLUE_OFF;
                            
                            previous_hello_time = get_usec();
                            break;

                        case SWITCH_ALPHA_:
                            switch_alpha = !switch_alpha;
                            speak(switch_alpha ? "alpha true" : "alpha false");
                            break;
                            
                        case SWITCH_BRAVO_:
                            switch_bravo = !switch_bravo;
                            speak(switch_bravo ? "bravo true" : "bravo false");
                            break;
                            
                        case SWITCH_CHARLIE_:
                            switch_charlie = !switch_charlie;
                            speak(switch_charlie ? "charlie true" : "charlie false");
                            break;
                            
                        case SWITCH_DELTA_:
                            switch_delta = !switch_delta;
                            speak(switch_delta ? "delta true" : "delta false");
                            break;
                            
                        case SWITCH_ECHO_:
                            switch_echo = !switch_echo;
                            speak(switch_echo ? "echo true" : "echo false");
                            break;
                            
                        case SWITCH_FOXTROT_:
                            switch_foxtrot = !switch_foxtrot;
                            speak(switch_foxtrot ? "foxtrot true" : "foxtrot false");
                            break;
                            
                        case STATUS_ALPHA_:
                            speak(switch_alpha ? "alpha true" : "alpha false");
                            break;
                            
                        case STATUS_BRAVO_:
                            speak(switch_bravo ? "bravo true" : "bravo false");
                            break;
                            
                        case STATUS_CHARLIE_:
                            speak(switch_charlie ? "charlie true" : "charlie false");
                            break;
                            
                        case STATUS_DELTA_:
                            speak(switch_delta ? "delta true" : "delta false");
                            break;
                            
                        case STATUS_ECHO_:
                            speak(switch_echo ? "echo true" : "echo false");
                            break;
                            
                        case STATUS_FOXTROT_:
                            speak(switch_foxtrot ? "foxtrot true" : "foxtrot false");
                            break;
                            
                        case VOLUME_UP_:
                        case VOLUME__UP_:
                            if (current_speaker_level < MAX_SPEAKER_LEVEL) {
                                set_speaker_level(current_speaker_level + 1);
                                say_speaker_level();
                                
                            } else {
                                speak("at maximum volume");
                            }
                            break;
                            
                        case VOLUME_DOWN_:
                        case VOLUME__DOWN_:
                            if (current_speaker_level > MIN_SPEAKER_LEVEL) {
                                set_speaker_level(current_speaker_level - 1);
                                say_speaker_level();

                            } else {
                                speak("at minimum volume");
                            }
                            break;
                            
                        case RESET_VOLUME_:
                            set_speaker_level(0);
                            say_speaker_level();
                            break;
                            
                        default:
                            fprintf(stderr, "Not handled: \"%s\"\n", index_to_phrase(cmd));
                            //speak(index_to_phrase(cmd));
                            break;
                    }
                }
                
                woke_time = get_usec();

                if (cmd != GREATER_THAN && cmd != ASTERISK) {
                    if (pending_shutdown && cmd != SHUT_DOWN_COMPUTER_) pending_shutdown = false;
                    
                    if (pending_exit && cmd != EXIT_PROGRAM_) pending_shutdown = false;
                }
            }

            if (confidence >= 0.0) {
                fprintf(stderr, "read \"%s\" [%d] (%4.2f)\n", buf_p, cmd, confidence);

            } else {
                fprintf(stderr, "read \"%s\" [%d]\n", buf, cmd);
            }
        }
        
        if (woke && !recording && get_usec() > woke_time + STAY_WOKE_TIME) {
            woke = false;
        }
        
        // set LEDs as needed
        if (recording) {
            if (rgb_led_state != RGB_RECORDING || blue_leds_state != BLUE_OFF) {
                fprintf(stderr, "set RGB_RECORDING BLUE_OFFE\n");

                put_message("X0 L0 L1 L2 L3 L4 L5 R0 G0 B0 C0 G255 T500 G0 T200 P0 ");
                rgb_led_state = RGB_RECORDING;
                blue_leds_state = BLUE_OFF;
            }

        } else if (!woke && distance <= WAKE_DISTANCE) {
            // show distance
            if (blue_leds_state != DISTANCE ||
                strcmp(blue_leds_for_distance(previous_distance), blue_leds_for_distance(distance)) != 0) {
                // fprintf(stderr, "set RGB_OFF DISTANCE\n");
                
                put_message("X0 ");
                put_message(blue_leds_for_distance(distance));
                blue_leds_state = DISTANCE;
                rgb_led_state = RGB_OFF;
                previous_distance = distance;
            }

        } else if (show_message_alert) {
            // priority over others
            if (rgb_led_state != RGB_MESSAGE || blue_leds_state != BLUE_MESSAGE) {
                fprintf(stderr, "set RGB_MESSAGE BLUE_MESSAGE\n");

                put_message("X0 L0 L1 L2 L3 L4 L5 R0 G0 B0 C0 H0 H5 R255 T350 L0 L5 R0 T700 P0 ");
                rgb_led_state = RGB_MESSAGE;
                blue_leds_state = BLUE_MESSAGE;
            }

        } else if (!woke) {
            if (blue_leds_state == BLUE_WOKE) all_off();
            
            if (show_air_quality) {
                if (rgb_led_state != AIR_QUALITY || air_quality != previous_air_quality) {
                    fprintf(stderr, "show_air_quality previous %d new %d\n", previous_air_quality, air_quality);
                    
                    put_message("X0 ");
                    
                    if (air_quality < 0) {
                        flash_color("R0 B100 G0 ");
                        
                    } else {
                        pulse_color(high_pulse_color_for_aqi(air_quality), low_pulse_color_for_aqi(air_quality));
                    }
                    
                    blue_leds_state = BLUE_OFF;
                    rgb_led_state = AIR_QUALITY;
                    previous_air_quality = air_quality;
                }
                
            } else {
                // show distance
                if (blue_leds_state != DISTANCE ||
                        strcmp(blue_leds_for_distance(previous_distance), blue_leds_for_distance(distance)) != 0) {
                    // fprintf(stderr, "set RGB_OFF DISTANCE\n");
                    
                    put_message("X1 R0 G0 B0 ");
                    put_message(blue_leds_for_distance(distance));
                    blue_leds_state = DISTANCE;
                    rgb_led_state = RGB_OFF;
                    previous_distance = distance;
                }
            }
            
        } else {
            if (show_air_quality) {
                if (rgb_led_state != AIR_QUALITY || air_quality != previous_air_quality) {
                    // fprintf(stderr, "set AIR_QUALITY BLUE_WOKE\n");
                    
                    put_message("X0 ");
                    wake_blue_leds();
                    
                    if (air_quality < 0) {
                        flash_color("R0 B100 G0 ");
                        
                    } else {
                        pulse_color(high_pulse_color_for_aqi(air_quality), low_pulse_color_for_aqi(air_quality));
                    }

                    rgb_led_state = AIR_QUALITY;
                    previous_air_quality = air_quality;
                }
                
            } else if (show_distance) {
                if (blue_leds_state != DISTANCE || distance != previous_distance) {
                    // fprintf(stderr, "set RGB_OFF DISTANCE\n");

                    put_message("X1 R0 G0 B0 ");
                    put_message(blue_leds_for_distance(distance));
                    blue_leds_state = DISTANCE;
                    rgb_led_state = RGB_OFF;
                    previous_distance = distance;
                }

            } else if (blue_leds_state != BLINKING) {
                // show woke
                if (blue_leds_state != BLUE_WOKE) {
                    // fprintf(stderr, "set RGB_OFF BLUE_WOKE\n");
                    all_off();
                    wake_blue_leds();
                    rgb_led_state = RGB_OFF;
                }
            }


        }
    }

    unmute_speaker();
    unmute_mic();
    
    close_mute();
    close_listen();
    close_play();
    close_sound();

    all_off();
    close_arduino();
    
    close_sonar();
    close_parse_web();

    return 0;
}
