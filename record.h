//
//  record.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef record_h
#define record_h

void start_recording(const char *path);
void stop_recording(void);

void rw(void);
void ro(void);

int message_count(void);

#endif /* record_h */
