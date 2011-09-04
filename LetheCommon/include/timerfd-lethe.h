#ifndef _LETHE_EVENTFD_H
#define _LETHE_EVENTFD_H

#include <sys/ioctl.h>

// IOCTL operations that may be performed on the lethe-timerfd device
#define TIMERFD_LETHE_MAJOR 246

#define TFD_SET_ABSOLUTE_TIME _IOW(TIMERFD_LETHE_MAJOR, 0, unsigned long)
#define TFD_SET_RELATIVE_TIME _IOW(TIMERFD_LETHE_MAJOR, 1, unsigned long)
#define TFD_SET_PERIODIC_TIME _IOW(TIMERFD_LETHE_MAJOR, 2, unsigned long)
#define TFD_SET_WAITREAD_MODE _IOW(TIMERFD_LETHE_MAJOR, 3, bool)
#define TFD_SET_ERROR _IOW(TIMERFD_LETHE_MAJOR, 4, bool)

#endif
