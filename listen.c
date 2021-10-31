//
//  listen.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "listen.h"

const char *listen_dir = "/home/pi/Projects/listen";
const char *listen_path = "/home/pi/Projects/listen/listen";

void fail_exit(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

pid_t listen_pid = 0;

int init_listen(int *listen_in_fd)
{
    const int READ_END = 0;
    const int WRITE_END = 1;
    
    int pipe_fds_listen_out[2];
    int pipe_fds_listen_in[2];

    if (pipe(pipe_fds_listen_out) == -1) fail_exit("pipe fail");
    if (pipe(pipe_fds_listen_in) == -1) fail_exit("pipe fail");

    listen_pid = fork();
    
    if (listen_pid == -1) {
        fail_exit("fork fail");
        
    } else if (listen_pid == 0) {
        if (dup2(pipe_fds_listen_out[WRITE_END], STDOUT_FILENO) == -1) fail_exit("dup2 fail");
        if (dup2(pipe_fds_listen_in[READ_END], STDIN_FILENO) == -1) fail_exit("dup2 fail");

        close(pipe_fds_listen_out[WRITE_END]);
        close(pipe_fds_listen_out[READ_END]);
        
        close(pipe_fds_listen_in[WRITE_END]);
        close(pipe_fds_listen_in[READ_END]);

        if (chdir(listen_dir) != 0) {
        fprintf(stderr, "fail chdir\n");
           exit(1);
        }
 
        execl(listen_path, "listen", NULL);
        
        fail_exit("execl fail");
    }
    
    close(pipe_fds_listen_out[WRITE_END]);
    close(pipe_fds_listen_in[READ_END]);

    *listen_in_fd = pipe_fds_listen_in[WRITE_END];
    
    return pipe_fds_listen_out[READ_END];
}

void close_listen(void)
{
    if (listen_pid > 0) kill(listen_pid, SIGTERM);
    listen_pid = 0;
}
