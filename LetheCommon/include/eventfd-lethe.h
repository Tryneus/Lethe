#ifndef _LETHE_EVENTFD_H
#define _LETHE_EVENTFD_H

#include <sys/ioctl.h>

// IOCTL operations that may be performed on the lethe-eventfd device
#define EVENTFD_LETHE_MAJOR 245

#define EFD_SET_WAITREAD_MODE _IOW(EVENTFD_LETHE_MAJOR, 0, bool)
#define EFD_SET_SEMAPHORE_MODE _IOW(EVENTFD_LETHE_MAJOR, 1, unsigned long)
#define EFD_SET_MUTEX_MODE _IOW(EVENTFD_LETHE_MAJOR, 2, unsigned long)
#define EFD_SET_EVENT_MODE _IOW(EVENTFD_LETHE_MAJOR, 3, unsigned long)
#define EFD_SET_MAX_VALUE _IOW(EVENTFD_LETHE_MAJOR, 4, unsigned long)
#define EFD_SET_ERROR _IOW(EVENTFD_LETHE_MAJOR, 5, bool)

#endif
