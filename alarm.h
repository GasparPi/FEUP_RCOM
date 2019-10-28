#pragma once
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define MAX_TIMEOUT 3

extern int alarmFlag;
extern int numRetry;

void alarmHandler(int signal);
void setAlarm(unsigned int seconds);
void stopAlarm();
