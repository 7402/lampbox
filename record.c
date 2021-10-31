//
//  record.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "record.h"
#include "listen.h"
#include "mute.h"

int record_pid = -1;

void start_recording(const char *path)
{
    record_pid = fork();
    
    if (record_pid == -1) {
        fail_exit("fork fail");
        
    } else if (record_pid == 0) {
        rw();
 
        execl("/usr/bin/arecord", "arecord", "-q", "-f", "S16_LE", "-r", "44100", path, NULL);
        fail_exit("execl fail");
        
    }
}

void stop_recording(void)
{
    if (record_pid > 0) {
        fprintf(stderr, "kill arecord\n");
        kill(record_pid, SIGINT);
        int wait_status;
        fprintf(stderr, "wait term arecord\n");
        waitpid(record_pid, &wait_status, 0);
        ro();
        fprintf(stderr, "arecord completed\n");
    }
}

void rw(void)
{
    int status = system("sudo mount -o remount,rw / ; sudo mount -o remount,rw /boot");
    system("PROMPT_COMMAND=set_bash_prompt");
    fprintf(stderr, "rw status = %d\n", status);
}

void ro(void)
{
    int status = system("sudo mount -o remount,ro / ; sudo mount -o remount,ro /boot");
    system("PROMPT_COMMAND=set_bash_prompt");
    fprintf(stderr, "ro status = %d\n", status);
}
