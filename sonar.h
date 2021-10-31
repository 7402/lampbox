//
//  sonar.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef sonar_h
#define sonar_h

void init_sonar(void);
void close_sonar(void);

extern const unsigned long long MSEC; // 1 msec in usec

unsigned long long get_usec(void);
void loop_sleep(unsigned long long usec);

double get_distance(void);

#endif /* sonar_h */
