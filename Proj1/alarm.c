#include "alarm.h"

void alarmHandler(int signal) {
  if(signal != SIGALRM)
    return;

  printf("Alarm: %d\n", dataLink.numRetries + 1);
	dataLink.alarmFlag = 1;
	dataLink.numRetries++;
  dataLink.stats.timeouts++;
}

void setAlarm() {
  // set sigaction struct
	struct sigaction sa;
	sa.sa_handler = &alarmHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGALRM, &sa, NULL);

	dataLink.alarmFlag = 0;

	alarm(dataLink.timeout); // install alarm
}

void stopAlarm() {
  // unset sigaction struct
	struct sigaction action;
	action.sa_handler = NULL;

	sigaction(SIGALRM, &action, NULL);

	alarm(0); // uninstall alarm
}
