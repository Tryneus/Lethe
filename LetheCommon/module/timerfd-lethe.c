/*
 *  fs/timerfd.c
 *
 *  Copyright (C) 2007  Davide Libenzi <davidel@xmailserver.org>
 *
 *
 *  Thanks to Thomas Gleixner for code reviews and useful comments.
 *
 */

#include <linux/file.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/anon_inodes.h>
#include "timerfd-internal.h"
#include <linux/module.h>

struct timerfd_ctx
{
  struct hrtimer timer;
  wait_queue_head_t wqh;
  ktime_t interval;
  u64 ticks;
  int expired;
  bool waitread;
  bool error;
};

void (*__wake_up_locked_keyPtr) (wait_queue_head_t *q, unsigned int mode, void *key); // offset from task_nice
extern int task_nice(const struct task_struct *p);

/*
 * This gets called when the timer event triggers. We set the "expired"
 * flag, but we do not re-arm the timer (in case it's necessary,
 * interval.tv64 != 0) until the timer is accessed.
 */
enum hrtimer_restart timerfd_callback(struct hrtimer *timer)
{
  struct timerfd_ctx *ctx = container_of(timer, struct timerfd_ctx, timer);
  unsigned long flags;

  spin_lock_irqsave(&ctx->wqh.lock, flags);
  ctx->expired = 1;
  ctx->ticks++;
  if (waitqueue_active(&ctx->wqh))
    __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLIN));
  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return HRTIMER_NORESTART;
}

int timerfd_setup(struct timerfd_ctx* ctx, const struct timespec* timer, int clockid, int mode)
{
  ktime_t texp;

  timerfd_lock_and_cancel(ctx);

  hrtimer_init(&ctx->timer, clockid, mode);
  texp = timespec_to_ktime(*timer);
  hrtimer_set_expires(&ctx->timer, texp);
  if (texp.tv64 != 0)
    hrtimer_start(&ctx->timer, texp, mode);

  ctx->timer.function = timerfd_callback;
  ctx->expired = 0;
  ctx->ticks = 0;

  spin_unlock_irq(&ctx->wqh.lock);

  return 0;
}

int timerfd_release(struct inode* inode, struct file* file)
{
  struct timerfd_ctx *ctx = file->private_data;

  hrtimer_cancel(&ctx->timer);
  wake_up_poll(&ctx->wqh, POLLHUP);
  module_put(THIS_MODULE);
  kfree(ctx);
  return 0;
}

unsigned int timerfd_poll(struct file *file, poll_table *wait)
{
  struct timerfd_ctx *ctx = file->private_data;
  unsigned int events = 0;
  unsigned long flags;

  poll_wait(file, &ctx->wqh, wait);

  spin_lock_irqsave(&ctx->wqh.lock, flags);
  if (ctx->error)
    events |= POLLERR;
  else if (ctx->ticks)
  {
    events |= POLLIN;

    if(ctx->waitread)
      (void) timerfd_do_read(ctx);
  }
  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return events;
}

ssize_t timerfd_do_read(struct timerfd_ctx* ctx)
{
  ssize_t ticks = ctx->ticks;

  if (ctx->expired && ctx->interval.tv64)
  {
    ticks += hrtimer_forward_now(&ctx->timer, ctx->interval) - 1;
    hrtimer_restart(&ctx->timer);
  }

  ctx->expired = 0;
  ctx->ticks = 0;

  return ticks;
}

ssize_t timerfd_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  struct timerfd_ctx *ctx = file->private_data;
  ssize_t res;
  u64 ticks = 0;

  if (count < sizeof(ticks))
    return -EINVAL;

  spin_lock_irq(&ctx->wqh.lock);
  if (ctx->ticks)
    ticks = timerfd_do_read(ctx);
  spin_unlock_irq(&ctx->wqh.lock);

  if (ticks)
    res = put_user(ticks, (u64 __user *) buf) ? -EFAULT: sizeof(ticks);
  else
    res = -EAGAIN;

  return res;
}

static const struct file_operations timerfd_fops =
{
  .open           = timerfd_open,
  .unlocked_ioctl = timerfd_ioctl,
  .poll           = timerfd_poll,
  .read           = timerfd_read,
  .release        = timerfd_release,
};

void timerfd_lock_and_cancel(struct timerfd_ctx* ctx)
{
  while(true)
  {
    spin_lock_irq(&ctx->wqh.lock);
    if (hrtimer_try_to_cancel(&ctx->timer) >= 0)
      return;
    spin_unlock_irq(&ctx->wqh.lock);
    cpu_relax();
  }
}

long timerfd_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  struct timerfd_ctx* ctx = file->private_data;
  struct timespec timeout;
  int res = 0;

  switch(ioctl_num)
  {
  case TFD_SET_RELATIVE_TIME:
    if (copy_from_user(&timeout, (void*) ioctl_param, sizeof(timeout)))
      res = -EFAULT;
    else
      timerfd_setup(ctx, &timeout, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    break;

  case TFD_SET_ABSOLUTE_TIME:
    if (copy_from_user(&timeout, (void*) ioctl_param, sizeof(timeout)))
      res = -EFAULT;
    else
      timerfd_setup(ctx, &timeout, CLOCK_REALTIME, HRTIMER_MODE_ABS);
    break;

  case TFD_SET_PERIODIC_TIME:
    if (copy_from_user(&timeout, (void*) ioctl_param, sizeof(timeout)))
      res = -EFAULT;
    else
    {
      timerfd_setup(ctx, &timeout, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
      ctx->interval = timespec_to_ktime(timeout);
    }
    break;

  case TFD_SET_WAITREAD_MODE:
    spin_lock_irq(&ctx->wqh.lock);
    ctx->waitread = (ioctl_param != 0);
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case TFD_SET_ERROR:
    spin_lock_irq(&ctx->wqh.lock);
    ctx->error = (ioctl_param != 0);
    if (ctx->error && waitqueue_active(&ctx->wqh))
      __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLERR));
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  default:
    res = -EINVAL;
    break;
  }

  return res;
}

int timerfd_open(struct inode* inode, struct file* file)
{
  struct timerfd_ctx* ctx;

  // Increment the use of this module
  if(!try_module_get(THIS_MODULE))
  {
    return -ENODEV;
  }

  ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);

  if (!ctx)
  {
    module_put(THIS_MODULE);
    return -ENOMEM;
  }

  file->private_data = ctx;
  init_waitqueue_head(&ctx->wqh);
  hrtimer_init(&ctx->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

  return 0;
}

int init_module()
{
  int res;

  if(!getExternals())
  {
    printk(KERN_INFO "timerfd-lethe failed to find necessary external symbols");
    return -EINVAL;
  }

  res = register_chrdev(TIMERFD_LETHE_MAJOR, "timerfd-lethe", &timerfd_fops);

  if(res < 0)
  {
    printk(KERN_INFO "Failed to register device number");
    return res;
  }

  printk(KERN_INFO "Successfully installed timerfd-lethe module");
  return 0;
}

void cleanup_module()
{
  unregister_chrdev(TIMERFD_LETHE_MAJOR, "timerfd-lethe");

  printk(KERN_INFO "Unloaded timerfd-lethe module");
}

bool isMatchAtEnd(char* str, char* substr)
{
  char* foundStr = strstr(str, substr);

  if(foundStr != NULL && foundStr[strlen(substr)] == '\0')
    return true;

  return false;
}

struct file* file_open(const char* path, int flags, int rights)
{
  struct file* filp = NULL;
  mm_segment_t oldfs;
  int err = 0;

  oldfs = get_fs();
  set_fs(get_ds());
  filp = filp_open(path, flags, rights);
  set_fs(oldfs);
  if(IS_ERR(filp))
  {
    err = PTR_ERR(filp);
    return NULL;
  }
  return filp;
}

void file_close(struct file* file)
{
  filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
  mm_segment_t oldfs;
  int ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_read(file, data, size, &offset);

  set_fs(oldfs);
  return ret;
}

void* getAddress(char* line)
{
  unsigned int address = 0;
  unsigned int index = 0;
  unsigned int length;
  char* addressString = strsep(&line, " ");
  length = strlen(addressString);

  if(length > 8)
    return NULL;

  while(index < length)
  {
    if(addressString[index] >= 'a' && addressString[index] <= 'f')
      address |= (addressString[index] - 'a' + 10) << ((length - index - 1) * 4);
    else if(addressString[index] >= 'A' && addressString[index] <= 'F')
      address |= (addressString[index] - 'A' + 10) << ((length - index - 1) * 4);
    else if(addressString[index] >= '0' && addressString[index] <= '9')
      address |= (addressString[index] - '0') << ((length - index - 1) * 4);

    ++index;
  }

  return (void*)address;
}

bool getExternals()
{
  char buffer[401];
  char* buffPtr;
  char temp[401];
  ssize_t readBytes;
  char* str = NULL;
  char* lastStr = NULL;
  void* taskNice = NULL;
  void* wakeUpLockedKey = NULL;
  struct file* symsFile = file_open("/proc/kallsyms", O_RDONLY, 0);
  unsigned long long fileOffset;

  if(symsFile == NULL)
  {
    printk(KERN_INFO "Failed to open file");
    return false;
  }

  readBytes = file_read(symsFile, 0, buffer, 400);
  buffer[readBytes] = '\0';
  temp[0] = '\0';

  fileOffset = readBytes;

  while(readBytes > 0)
  {
    buffPtr = buffer;
    str = strsep(&buffPtr, "\n");

    if(strlen(temp) != 0)
    {
      strcat(temp, str);
      str = temp;
    }

    while(str != NULL)
    {
      if(isMatchAtEnd(str, " task_nice")) taskNice = getAddress(str);
      if(isMatchAtEnd(str, " __wake_up_locked_key")) wakeUpLockedKey = getAddress(str);

      lastStr = str;
      str = strsep(&buffPtr, "\n");
    }

    strcpy(temp, lastStr);

    readBytes = file_read(symsFile, fileOffset, buffer, 400);
    buffer[readBytes] = '\0';
    fileOffset += readBytes;
  }

  file_close(symsFile);

  if(taskNice == NULL ||
     wakeUpLockedKey == NULL)
    return false;

  __wake_up_locked_keyPtr = (void*)task_nice + ((void*)wakeUpLockedKey - (void*)taskNice);

  return true;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tryneus");
MODULE_DESCRIPTION("extended timerfd for lethe project");

