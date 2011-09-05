/*
 *  fs/eventfd.c
 *
 *  Copyright (C) 2007  Davide Libenzi <davidel@xmailserver.org>
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
#include <linux/anon_inodes.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/buffer_head.h>
#include <asm/current.h>
#include "eventfd-internal.h"

void (*__wake_up_locked_keyPtr) (wait_queue_head_t *q, unsigned int mode, void *key); // offset from task_nice
extern int task_nice(const struct task_struct *p);

/*
void logString(char* context, char* str)
{
  char buffer[500];
  buffer[0] = '\0';

  strcat(buffer, KERN_INFO);
  strcat(buffer, context);
  strcat(buffer, ": ");
  strcat(buffer, str);
  printk(buffer);
}

void logValue(char* context, unsigned int value, unsigned int base)
{
  char buffer[40];
  unsigned int index;
  buffer[39] = '\0';
  index = 38;

  if(value == 0)
    buffer[index--] = '0';

  while(value > 0)
  {
    if((value % base) > 9)
      buffer[index] = 'A' + (value % base) - 10;
    else
      buffer[index] = '0' + (value % base);

    --index;
    value /= base;
  }

  if(base == 16)
  {
    buffer[index--] = 'x';
    buffer[index--] = '0';
  }
  else if(base == 2)
  {
    buffer[index--] = 'b';
    buffer[index--] = '0';
  }

  logString(context, &buffer[index + 1]);
}
*/

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

bool getExternals(void)
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

struct eventfd_ctx {
  wait_queue_head_t wqh;
  /*
   * Every time that a write(2) is performed on an eventfd, the
   * value of the __u64 being written is added to "count" and a
   * wakeup is performed on "wqh". A read(2) will return the "count"
   * value to userspace, and will reset "count" to zero. The kernel
   * side eventfd_signal() also, adds to the "count" counter and
   * issue a wakeup.
   */
  __u64 count;

  /* Members added for eventfd-lethe extensions */
  enum eventfd_mode_t mode;
  bool error;
  bool waitread;

  /* Only used in semaphore mode */
  __u64 maxCount;

  /* Only used in mutex mode */
  pid_t pid;
  pid_t tid;
};

int eventfd_flush(struct file* file, fl_owner_t id)
{
  struct eventfd_ctx *ctx = file->private_data;

  // Make sure a mutex object isn't owned by the releasing thread
  if(ctx->mode == EFD_MUTEX_MODE)
  {
    pid_t pid = task_tgid_vnr(current);
    pid_t tid = task_pid_vnr(current);
    unsigned long flags;

    spin_lock_irqsave(&ctx->wqh.lock, flags);
    if(ctx->pid == pid && ctx->tid == tid) // TODO: only match the pid? - this should only be called when the fd is closed or the process exits
    {
      ctx->count = 0;
      ctx->error = true;
      if (waitqueue_active(&ctx->wqh))
        __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLERR));
    }
    spin_unlock_irqrestore(&ctx->wqh.lock, flags);
  }

  return 0;
}

int eventfd_release(struct inode *inode, struct file *file)
{
  struct eventfd_ctx *ctx = file->private_data;

  wake_up_poll(&ctx->wqh, POLLHUP);
  module_put(THIS_MODULE);
  kfree(ctx);
  return 0;
}

unsigned int eventfd_poll(struct file *file, poll_table *wait)
{
  struct eventfd_ctx *ctx = file->private_data;
  unsigned int events = 0;
  unsigned long flags;

  poll_wait(file, &ctx->wqh, wait);

  spin_lock_irqsave(&ctx->wqh.lock, flags);

  if (ctx->error)
    events |= POLLERR;
  else if (ctx->waitread)
  {
    __u64 readCount;
    eventfd_ctx_do_read(ctx, &readCount);

    if (readCount > 0)
      events |= POLLIN;
  }
  else
  {
    if(ctx->mode == EFD_MUTEX_MODE)
    {
      if(ctx->count == 0)
        events |= POLLIN;
    }
    else if(ctx->count > 0)
      events |= POLLIN;
  }

  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return events;
}

void eventfd_ctx_do_read(struct eventfd_ctx* ctx, __u64* cnt)
{
  if(ctx->mode == EFD_SEMAPHORE_MODE)
  {
    if(ctx->count > 0)
    {
      *cnt = 1;
      ctx->count -= 1;
    }
    else
      *cnt = 0;
  }
  else if(ctx->mode == EFD_MUTEX_MODE)
  {
    pid_t pid = task_tgid_vnr(current);
    pid_t tid = task_pid_vnr(current);

    if(ctx->count == 0)
    {
      *cnt = 1;
      ctx->count = 1;
      ctx->pid = pid;
      ctx->tid = tid;
    }
    else if(ctx->pid == pid && ctx->tid == tid)
    {
      // If locked by the same thread, accumulate more locks
      *cnt = 1;
      ctx->count += 1;
    }
    else
      *cnt = 0;
  }
  else // EFD_EVENT_MODE
  {
    if(ctx->count > 0)
    {
      *cnt = ctx->count;
      ctx->count = 0;
    }
    else
      *cnt = 0;
  }
}

ssize_t eventfd_read(struct file *file, char __user *buf, size_t count,
                            loff_t *ppos)
{
  struct eventfd_ctx *ctx = file->private_data;
  __u64 cnt;

  if (count < sizeof(cnt))
    return -EINVAL;

  spin_lock_irq(&ctx->wqh.lock);
  eventfd_ctx_do_read(ctx, &cnt);
  spin_unlock_irq(&ctx->wqh.lock);

  if (cnt == 0)
    return -EAGAIN;

  return put_user(cnt, (__u64 __user *) buf) ? -EFAULT : sizeof(cnt);
}

ssize_t eventfd_write(struct file *file,
                      const char __user *buf,
                      size_t count,
                      loff_t *ppos)
{
  struct eventfd_ctx *ctx = file->private_data;
  ssize_t res;
  __u64 cnt;

  if (count < sizeof(cnt))
    return -EINVAL;
  if (copy_from_user(&cnt, buf, sizeof(cnt)))
    return -EFAULT;
  if (cnt == ULLONG_MAX)
    return -EINVAL;

  spin_lock_irq(&ctx->wqh.lock);

  res = sizeof(cnt);
  if(ctx->mode == EFD_SEMAPHORE_MODE)
  {
    ctx->count += cnt;

    if(ctx->count > ctx->maxCount)
    {
      res = -EINVAL;
      ctx->count -= cnt;
    }
  }
  else if(ctx->mode == EFD_MUTEX_MODE)
  {
    // Only allow writes from the thread who did the read
    pid_t pid = task_tgid_vnr(current);
    pid_t tid = task_pid_vnr(current);

    if(cnt <= ctx->count && ctx->pid == pid && ctx->tid == tid)
      ctx->count -= cnt;
    else
      res = -EPERM;
  }
  else // EFD_EVENT_MODE
    ctx->count += cnt;

  if(res > 0 && waitqueue_active(&ctx->wqh))
    __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLIN));

  spin_unlock_irq(&ctx->wqh.lock);

  return res;
}

long eventfd_ioctl(struct file* file,
                   unsigned int ioctl_num,
                   unsigned long ioctl_param)
{
  struct eventfd_ctx *ctx = file->private_data;
  int retval = 0;

  switch(ioctl_num)
  {
  case EFD_SET_WAITREAD_MODE:
    spin_lock_irq(&ctx->wqh.lock);
    if (likely(!waitqueue_active(&ctx->wqh)))
    {
      ctx->waitread = (ioctl_param != 0);
    }
    else
      retval = -EINVAL;
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_SET_SEMAPHORE_MODE: // Sets the file to "semaphore" mode, has a settable upper limit for allowed number of locks, no thread checks, a read locks once
    spin_lock_irq(&ctx->wqh.lock);
    if (likely(!waitqueue_active(&ctx->wqh)))
    {
      ctx->mode = EFD_SEMAPHORE_MODE;
      ctx->count = ioctl_param;
      ctx->maxCount = ULLONG_MAX;
    }
    else
      retval = -EINVAL;
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_SET_MUTEX_MODE: // Sets the file to "mutex" mode, only one lock allowed, but allows multiple locks from the same thread, a read locks once, every successful lock must correspond to an unlock
    spin_lock_irq(&ctx->wqh.lock);
    if (likely(!waitqueue_active(&ctx->wqh)))
    {
      ctx->mode = EFD_MUTEX_MODE;
      if(ioctl_param > 0)
      {
        ctx->count = ioctl_param;
        ctx->pid = task_tgid_vnr(current);
        ctx->tid = task_pid_vnr(current);
      }
      else
      {
        ctx->count = 0;
        ctx->pid = 0;
        ctx->tid = 0;
      }
    }
    else
      retval = -EINVAL;
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_SET_EVENT_MODE: // Sets the file to "event" mode, a read clears the count entirely
    spin_lock_irq(&ctx->wqh.lock);
    if (likely(!waitqueue_active(&ctx->wqh)))
    {
      ctx->mode = EFD_EVENT_MODE;
      ctx->count = ioctl_param;
    }
    else
      retval = -EINVAL;
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_SET_MAX_VALUE: // Sets the maximum value, only valid for semaphore mode
    spin_lock_irq(&ctx->wqh.lock);
    if (likely(ctx->count <= ioctl_param && ctx->mode == EFD_SEMAPHORE_MODE))
      ctx->maxCount = ioctl_param;
    else
      retval = -EINVAL;
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_SET_ERROR: // Set the error field and wake up any waiting threads
    spin_lock_irq(&ctx->wqh.lock);
    ctx->error = (ioctl_param != 0);
    if (ctx->error && waitqueue_active(&ctx->wqh))
      __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLERR));
    spin_unlock_irq(&ctx->wqh.lock);
    break;

  case EFD_GET_MODE:
    retval = ctx->mode;
    break;

  default:
    retval = -EINVAL;
    break;
  }

  return retval;
}

const struct file_operations eventfd_fops = {
  .open           = eventfd_open,
  .unlocked_ioctl = eventfd_ioctl,
  .poll           = eventfd_poll,
  .read           = eventfd_read,
  .write          = eventfd_write,
  .flush          = eventfd_flush,
  .release        = eventfd_release,
};

int eventfd_open(struct inode* inode, struct file* file)
{
  struct eventfd_ctx* ctx;

  // Increment the use of this module
  if(!try_module_get(THIS_MODULE))
  {
    printk(KERN_INFO "eventfd try_module_get failed");
    return -ENODEV;
  }

  ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);

  if (!ctx)
  {
    printk(KERN_INFO "failed to allocate eventfd ctx");
    module_put(THIS_MODULE);
    return -ENOMEM;
  }

  init_waitqueue_head(&ctx->wqh);

  ctx->mode = EFD_EVENT_MODE;
  ctx->error = false;
  ctx->waitread = false;
  file->private_data = ctx;

  return 0;
}

int init_module(void)
{
  int res;

  if(!getExternals())
  {
    printk(KERN_INFO "eventfd-lethe failed to find necessary external symbols");
    return -EINVAL;
  }

  res = register_chrdev(EVENTFD_LETHE_MAJOR, "eventfd-lethe", &eventfd_fops);

  if(res < 0)
  {
    printk(KERN_INFO "Failed to register device number");
    return res;
  }

  printk(KERN_INFO "Successfully installed eventfd-lethe module");
  return 0;
}

void cleanup_module(void)
{
  unregister_chrdev(EVENTFD_LETHE_MAJOR, "eventfd-lethe");

  printk(KERN_INFO "Unloaded eventfd-lethe module");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tryneus");
MODULE_DESCRIPTION("extended eventfd for lethe project");

