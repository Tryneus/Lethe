/*
 *  include/linux/eventfd.h
 *
 *  Copyright (C) 2007  Davide Libenzi <davidel@xmailserver.org>
 *
 *  modified for Lethe
 *
 */

#ifndef _LETHE_EVENTFD_H
#define _LETHE_EVENTFD_H

#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/ioctl.h>

// Modes
enum eventfd_mode_t
{
  EFD_SEMAPHORE_MODE,
  EFD_MUTEX_MODE,
  EFD_EVENT_MODE
};

// IOCTL operations that may be performed on the lethe-eventfd device
#define EVENTFD_LETHE_MAJOR 245

#define EFD_SET_WAITREAD_MODE _IOW(EVENTFD_LETHE_MAJOR, 0, bool)
#define EFD_SET_SEMAPHORE_MODE _IOW(EVENTFD_LETHE_MAJOR, 1, unsigned long)
#define EFD_SET_MUTEX_MODE _IOW(EVENTFD_LETHE_MAJOR, 2, unsigned long)
#define EFD_SET_EVENT_MODE _IOW(EVENTFD_LETHE_MAJOR, 3, unsigned long)
#define EFD_SET_MAX_VALUE _IOW(EVENTFD_LETHE_MAJOR, 4, unsigned long)
#define EFD_SET_ERROR _IOW(EVENTFD_LETHE_MAJOR, 5, bool)
#define EFD_GET_MODE _IO(EVENTFD_LETHE_MAJOR, 6)

int init_module(void);
void cleanup_module(void);

int eventfd_open(struct inode* inode, struct file* file);
long eventfd_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param);
ssize_t eventfd_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
unsigned int eventfd_poll(struct file *file, poll_table *wait);
ssize_t eventfd_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
void eventfd_ctx_do_read(struct eventfd_ctx* ctx, __u64* cnt);
int eventfd_flush(struct file* file, fl_owner_t id);
void eventfd_free(struct kref *kref);
int eventfd_release(struct inode *inode, struct file *file);

#endif
