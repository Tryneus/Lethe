/*
 *  fs/timerfd.c
 *
 *  Copyright (C) 2007  Davide Libenzi <davidel@xmailserver.org>
 *  Thanks to Thomas Gleixner for code reviews and useful comments.
 *
 *  modified for lethe
 *
 */

#ifndef _LETHE_EVENTFD_H
#define _LETHE_EVENTFD_H

#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/ioctl.h>
#include <linux/time.h>

struct timerfd_ctx;

// IOCTL operations that may be performed on the lethe-timerfd device
#define TIMERFD_LETHE_MAJOR 246

#define TFD_SET_ABSOLUTE_TIME _IOW(TIMERFD_LETHE_MAJOR, 0, unsigned long)
#define TFD_SET_RELATIVE_TIME _IOW(TIMERFD_LETHE_MAJOR, 1, unsigned long)
#define TFD_SET_PERIODIC_TIME _IOW(TIMERFD_LETHE_MAJOR, 2, unsigned long)
#define TFD_SET_WAITREAD_MODE _IOW(TIMERFD_LETHE_MAJOR, 3, bool)
#define TFD_SET_ERROR _IOW(TIMERFD_LETHE_MAJOR, 4, bool)

int init_module(void);
void cleanup_module(void);

bool getExternals(void);
enum hrtimer_restart timerfd_callback(struct hrtimer *timer);

int timerfd_open(struct inode* inode, struct file* file);
int timerfd_setup(struct timerfd_ctx* ctx, const struct timespec* timer, int clockid, int mode);
long timerfd_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param);
void timerfd_lock_and_cancel(struct timerfd_ctx* ctx);
unsigned int timerfd_poll(struct file *file, poll_table *wait);
ssize_t timerfd_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t timerfd_do_read(struct timerfd_ctx* ctx);
int timerfd_release(struct inode *inode, struct file *file);

#endif
