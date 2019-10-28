#include "alarm.h"

void alarmHandler(int signal) {
  if(signal != SIGALRM)
    return;

  printf("Alarm: %d\n", numRetry + 1);
	alarmFlag = 1;
	numRetry++;

	//alarm(MAX_TIMEOUT);
}

void setAlarm(unsigned int seconds) {
  // set sigaction struct
	struct sigaction sa;
	sa.sa_handler = &alarmHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGALRM, &sa, NULL);

	alarmFlag = 0;

	alarm(seconds); // install alarm
}

void stopAlarm() {
  // unset sigaction struct
	struct sigaction action;
	action.sa_handler = NULL;

	sigaction(SIGALRM, &action, NULL);

	alarm(0); // uninstall alarm
}
