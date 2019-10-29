#pragma once
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "ll.h"

#define MAX_TIMEOUT 3

void alarmHandler(int signal);
void setAlarm();
void stopAlarm();
