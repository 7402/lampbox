//
//  listen.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef listen_h
#define listen_h

int init_listen(int *listen_in_fd);
void close_listen(void);

void fail_exit(const char *message);

#endif /* listen_h */
