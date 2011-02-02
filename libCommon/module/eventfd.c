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
#include <linux/kref.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/buffer_head.h>
#include "eventfd-internal.h"

// Since someone decided it would be a great idea to keep modules from accessing some symbols, attempt to find the locations of the symbols we want

// symbols needed: sys_call_table, alloc_fd, and __wake_up_locked_key
// alloc_fd - maybe use get_unused_fd ?
// __wake_up_locked_key - exported wake functions don't look right
// sys_call_table - any other way to add a new sys call

void** sys_call_tablePtr; // offset from register_kprobes
int (*alloc_fdPtr)(unsigned, unsigned); // offset from get_unused_fd
void (*__wake_up_locked_keyPtr) (wait_queue_head_t *q, unsigned int mode, void *key); // offset from task_nice

extern int task_nice(const struct task_struct *p);
extern int get_unused_fd(void);
extern int register_kprobes(struct kprobe **kps, int num);
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
  void* getUnusedFd = NULL;
  void* registerKprobes = NULL;
  void* sysCallTable = NULL;
  void* allocFd = NULL;
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
      if(isMatchAtEnd(str, " get_unused_fd")) getUnusedFd = getAddress(str);
      if(isMatchAtEnd(str, " register_kprobes")) registerKprobes = getAddress(str);
      if(isMatchAtEnd(str, " sys_call_table")) sysCallTable = getAddress(str);
      if(isMatchAtEnd(str, " alloc_fd")) allocFd = getAddress(str);
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
     getUnusedFd == NULL ||
     registerKprobes == NULL ||
     sysCallTable == NULL ||
     allocFd == NULL ||
     wakeUpLockedKey == NULL)
    return false;

  sys_call_tablePtr = (void*)register_kprobes + ((void*)sysCallTable - (void*)registerKprobes);
  alloc_fdPtr = (void*)get_unused_fd + ((void*)allocFd - (void*)getUnusedFd);
  __wake_up_locked_keyPtr = (void*)task_nice + ((void*)wakeUpLockedKey - (void*)taskNice);

  return true;
}

struct eventfd_ctx {
  struct kref kref;
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
  unsigned int flags;
};

/**
 * eventfd_signal - Adds @n to the eventfd counter.
 * @ctx: [in] Pointer to the eventfd context.
 * @n: [in] Value of the counter to be added to the eventfd internal counter.
 *          The value cannot be negative.
 *
 * This function is supposed to be called by the kernel in paths that do not
 * allow sleeping. In this function we allow the counter to reach the ULLONG_MAX
 * value, and we signal this as overflow condition by returining a POLLERR
 * to poll(2).
 *
 * Returns @n in case of success, a non-negative number lower than @n in case
 * of overflow, or the following error codes:
 *
 * -EINVAL    : The value of @n is negative.
 */
int eventfd_signal(struct eventfd_ctx *ctx, int n)
{
  unsigned long flags;

  if (n < 0)
          return -EINVAL;
  spin_lock_irqsave(&ctx->wqh.lock, flags);
  if (ULLONG_MAX - ctx->count < n)
    n = (int) (ULLONG_MAX - ctx->count);
  ctx->count += n;
  if (waitqueue_active(&ctx->wqh))
    __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLIN));
  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return n;
}

static void eventfd_free_ctx(struct eventfd_ctx *ctx)
{
  kfree(ctx);
}

static void eventfd_free(struct kref *kref)
{
  struct eventfd_ctx *ctx = container_of(kref, struct eventfd_ctx, kref);

  eventfd_free_ctx(ctx);
}

/**
 * eventfd_ctx_get - Acquires a reference to the internal eventfd context.
 * @ctx: [in] Pointer to the eventfd context.
 *
 * Returns: In case of success, returns a pointer to the eventfd context.
 */
struct eventfd_ctx *eventfd_ctx_get(struct eventfd_ctx *ctx)
{
  kref_get(&ctx->kref);
  return ctx;
}

/**
 * eventfd_ctx_put - Releases a reference to the internal eventfd context.
 * @ctx: [in] Pointer to eventfd context.
 *
 * The eventfd context reference must have been previously acquired either
 * with eventfd_ctx_get() or eventfd_ctx_fdget()).
 */
void eventfd_ctx_put(struct eventfd_ctx *ctx)
{
  kref_put(&ctx->kref, eventfd_free);
}

static int eventfd_release(struct inode *inode, struct file *file)
{
  struct eventfd_ctx *ctx = file->private_data;

  wake_up_poll(&ctx->wqh, POLLHUP);
  eventfd_ctx_put(ctx);
  return 0;
}

static void eventfd_ctx_do_read(struct eventfd_ctx *ctx, __u64 *cnt)
{
  *cnt = (ctx->flags & EFD_SEMAPHORE) ? 1 : ctx->count;
  ctx->count -= *cnt;
}

static unsigned int eventfd_poll(struct file *file, poll_table *wait)
{
  struct eventfd_ctx *ctx = file->private_data;
  unsigned int events = 0;
  unsigned long flags;

  poll_wait(file, &ctx->wqh, wait);

  spin_lock_irqsave(&ctx->wqh.lock, flags);
  if (ctx->count > 0) {
    if (ctx->flags & EFD_WAITREAD) {
      __u64 dummy;
      eventfd_ctx_do_read(ctx, &dummy);
      if (waitqueue_active(&ctx->wqh))
        __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLOUT));
    }
    events |= POLLIN;
  }
  if (ctx->count == ULLONG_MAX)
    events |= POLLERR;
  if (ULLONG_MAX - 1 > ctx->count)
    events |= POLLOUT;
  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return events;
}

/**
 * eventfd_ctx_remove_wait_queue - Read the current counter and removes wait queue.
 * @ctx: [in] Pointer to eventfd context.
 * @wait: [in] Wait queue to be removed.
 * @cnt: [out] Pointer to the 64bit conter value.
 *
 * Returns zero if successful, or the following error codes:
 *
 * -EAGAIN      : The operation would have blocked.
 *
 * This is used to atomically remove a wait queue entry from the eventfd wait
 * queue head, and read/reset the counter value.
 */
int eventfd_ctx_remove_wait_queue(struct eventfd_ctx *ctx, wait_queue_t *wait,
                                  __u64 *cnt)
{
  unsigned long flags;

  spin_lock_irqsave(&ctx->wqh.lock, flags);
  eventfd_ctx_do_read(ctx, cnt);
  __remove_wait_queue(&ctx->wqh, wait);
  if (*cnt != 0 && waitqueue_active(&ctx->wqh))
    __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLOUT));
  spin_unlock_irqrestore(&ctx->wqh.lock, flags);

  return *cnt != 0 ? 0 : -EAGAIN;
}

/**
 * eventfd_ctx_read - Reads the eventfd counter or wait if it is zero.
 * @ctx: [in] Pointer to eventfd context.
 * @no_wait: [in] Different from zero if the operation should not block.
 * @cnt: [out] Pointer to the 64bit conter value.
 *
 * Returns zero if successful, or the following error codes:
 *
 * -EAGAIN      : The operation would have blocked but @no_wait was nonzero.
 * -ERESTARTSYS : A signal interrupted the wait operation.
 *
 * If @no_wait is zero, the function might sleep until the eventfd internal
 * counter becomes greater than zero.
 */
ssize_t eventfd_ctx_read(struct eventfd_ctx *ctx, int no_wait, __u64 *cnt)
{
  ssize_t res;
  DECLARE_WAITQUEUE(wait, current);

  spin_lock_irq(&ctx->wqh.lock);
  *cnt = 0;
  res = -EAGAIN;
  if (ctx->count > 0)
    res = 0;
  else if (!no_wait) {
    __add_wait_queue(&ctx->wqh, &wait);
    for (;;) {
      set_current_state(TASK_INTERRUPTIBLE);
      if (ctx->count > 0) {
        res = 0;
        break;
      }
      if (signal_pending(current)) {
        res = -ERESTARTSYS;
        break;
      }
      spin_unlock_irq(&ctx->wqh.lock);
      schedule();
      spin_lock_irq(&ctx->wqh.lock);
    }
    __remove_wait_queue(&ctx->wqh, &wait);
    __set_current_state(TASK_RUNNING);
  }
  if (likely(res == 0)) {
    eventfd_ctx_do_read(ctx, cnt);
    if (waitqueue_active(&ctx->wqh))
      __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLOUT));
  }
  spin_unlock_irq(&ctx->wqh.lock);

  return res;
}

static ssize_t eventfd_read(struct file *file, char __user *buf, size_t count,
                            loff_t *ppos)
{
  struct eventfd_ctx *ctx = file->private_data;
  ssize_t res;
  __u64 cnt;

  if (count < sizeof(cnt))
    return -EINVAL;
  res = eventfd_ctx_read(ctx, file->f_flags & O_NONBLOCK, &cnt);
  if (res < 0)
    return res;

  return put_user(cnt, (__u64 __user *) buf) ? -EFAULT : sizeof(cnt);
}

static ssize_t eventfd_write(struct file *file, const char __user *buf, size_t count,
                             loff_t *ppos)
{
  struct eventfd_ctx *ctx = file->private_data;
  ssize_t res;
  __u64 ucnt;
  DECLARE_WAITQUEUE(wait, current);

  if (count < sizeof(ucnt))
    return -EINVAL;
  if (copy_from_user(&ucnt, buf, sizeof(ucnt)))
    return -EFAULT;
  if (ucnt == ULLONG_MAX)
    return -EINVAL;
  spin_lock_irq(&ctx->wqh.lock);
  res = -EAGAIN;
  if (ULLONG_MAX - ctx->count > ucnt)
    res = sizeof(ucnt);
  else if (!(file->f_flags & O_NONBLOCK)) {
    __add_wait_queue(&ctx->wqh, &wait);
    for (res = 0;;) {
      set_current_state(TASK_INTERRUPTIBLE);
      if (ULLONG_MAX - ctx->count > ucnt) {
        res = sizeof(ucnt);
        break;
      }
      if (signal_pending(current)) {
        res = -ERESTARTSYS;
        break;
      }
      spin_unlock_irq(&ctx->wqh.lock);
      schedule();
      spin_lock_irq(&ctx->wqh.lock);
    }
    __remove_wait_queue(&ctx->wqh, &wait);
    __set_current_state(TASK_RUNNING);
  }
  if (likely(res > 0)) {
    ctx->count += ucnt;
    if (waitqueue_active(&ctx->wqh))
      __wake_up_locked_keyPtr(&ctx->wqh, TASK_NORMAL, (void *) (POLLIN));
  }
  spin_unlock_irq(&ctx->wqh.lock);

  return res;
}

static const struct file_operations eventfd_fops = {
  .release        = eventfd_release,
  .poll           = eventfd_poll,
  .read           = eventfd_read,
  .write          = eventfd_write,
};

/**
 * eventfd_fget - Acquire a reference of an eventfd file descriptor.
 * @fd: [in] Eventfd file descriptor.
 *
 * Returns a pointer to the eventfd file structure in case of success, or the
 * following error pointer:
 *
 * -EBADF    : Invalid @fd file descriptor.
 * -EINVAL   : The @fd file descriptor is not an eventfd file.
 */
struct file *eventfd_fget(int fd)
{
  struct file *file;

  file = fget(fd);
  if (!file)
    return ERR_PTR(-EBADF);
  if (file->f_op != &eventfd_fops) {
    fput(file);
    return ERR_PTR(-EINVAL);
  }

  return file;
}

/**
 * eventfd_ctx_fdget - Acquires a reference to the internal eventfd context.
 * @fd: [in] Eventfd file descriptor.
 *
 * Returns a pointer to the internal eventfd context, otherwise the error
 * pointers returned by the following functions:
 *
 * eventfd_fget
 */
struct eventfd_ctx *eventfd_ctx_fdget(int fd)
{
  struct file *file;
  struct eventfd_ctx *ctx;

  file = eventfd_fget(fd);
  if (IS_ERR(file))
    return (struct eventfd_ctx *) file;
  ctx = eventfd_ctx_get(file->private_data);
  fput(file);

  return ctx;
}

/**
 * eventfd_ctx_fileget - Acquires a reference to the internal eventfd context.
 * @file: [in] Eventfd file pointer.
 *
 * Returns a pointer to the internal eventfd context, otherwise the error
 * pointer:
 *
 * -EINVAL   : The @fd file descriptor is not an eventfd file.
 */
struct eventfd_ctx *eventfd_ctx_fileget(struct file *file)
{
  if (file->f_op != &eventfd_fops)
    return ERR_PTR(-EINVAL);

  return eventfd_ctx_get(file->private_data);
}

/**
 * eventfd_file_create - Creates an eventfd file pointer.
 * @count: Initial eventfd counter value.
 * @flags: Flags for the eventfd file.
 *
 * This function creates an eventfd file pointer, w/out installing it into
 * the fd table. This is useful when the eventfd file is used during the
 * initialization of data structures that require extra setup after the eventfd
 * creation. So the eventfd creation is split into the file pointer creation
 * phase, and the file descriptor installation phase.
 * In this way races with userspace closing the newly installed file descriptor
 * can be avoided.
 * Returns an eventfd file pointer, or a proper error pointer.
 */
struct file *eventfd_file_create(unsigned int count, int flags)
{
  struct file *file;
  struct eventfd_ctx *ctx;

  /* Check the EFD_* constants for consistency.  */
  BUILD_BUG_ON(EFD_CLOEXEC != O_CLOEXEC);
  BUILD_BUG_ON(EFD_NONBLOCK != O_NONBLOCK);

  if (flags & ~EFD_FLAGS_SET)
  {
    return ERR_PTR(-EINVAL);
  }

  ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
  if (!ctx)
  {
    return ERR_PTR(-ENOMEM);
  }

  kref_init(&ctx->kref);
  init_waitqueue_head(&ctx->wqh);
  ctx->count = count;
  ctx->flags = flags;

  file = anon_inode_getfile("[eventfd]", &eventfd_fops, ctx,
                            O_RDWR | (flags & EFD_SHARED_FCNTL_FLAGS));
  if (IS_ERR(file))
    eventfd_free_ctx(ctx);

  return file;
}

asmlinkage long mod_eventfd2(unsigned int count, int flags)
{
  int fd, error;
  struct file *file;

  error = alloc_fdPtr(0, (flags & EFD_SHARED_FCNTL_FLAGS));

  if (error < 0)
    return error;
  fd = error;

  file = eventfd_file_create(count, flags);
  if (IS_ERR(file)) {
    error = PTR_ERR(file);
    goto err_put_unused_fd;
  }
  fd_install(fd, file);
  return fd;

err_put_unused_fd:
  put_unused_fd(fd);
  return error;
}

asmlinkage long mod_eventfd(unsigned int count)
{
  return mod_eventfd2(count, 0);
}

// Additions made to make this an LKM

unsigned int* pageTable;
unsigned int pindex;
unsigned long old_page_table_entry;
asmlinkage int (*original_eventfd) (unsigned int);
asmlinkage int (*original_eventfd2) (unsigned int, int);
unsigned int _cr3;
unsigned int _cr4;

bool replacedSyscalls;

int init_module(void)
{
  unsigned int dindex;
  unsigned int* pageDir;
  replacedSyscalls = false;
  _cr3 = 0;
  _cr4 = 0;

  if(getExternals())
  {
    printk(KERN_INFO "Successfully found symbols");

    // Change the sys_call_table page to writable
    // Copied from http://www.cs.usfca.edu/~cruse/cs635/newcall.c

    asm(" mov %%cr4, %%eax \n mov %%eax, _cr4 " ::: "ax" );
    asm(" mov %%cr3, %%eax \n mov %%eax, _cr3 " ::: "ax" );

    if ( (_cr4 >> 5) & 1 )
    {
      printk(KERN_INFO "processor is using Page-Address Extensions");
      return -ENOSYS;
    }

    dindex = ((int)sys_call_tablePtr >> 22) & 0x3FF;
    pindex = ((int)sys_call_tablePtr >> 12) & 0x3FF;

    pageDir = phys_to_virt( _cr3 & ~0xFFF );
    pageTable = phys_to_virt( pageDir[dindex] & ~0xFFF);

    old_page_table_entry = pageTable[pindex];
    pageTable[pindex] |= 2; // Make writable

    original_eventfd = sys_call_tablePtr[__NR_eventfd];
    original_eventfd2 = sys_call_tablePtr[__NR_eventfd2];
    sys_call_tablePtr[__NR_eventfd] = mod_eventfd;
    sys_call_tablePtr[__NR_eventfd2] = mod_eventfd2;
    replacedSyscalls = true;
  }
  else
  {
    printk(KERN_INFO "Failed to get symbols");
    return -EINVAL;
  }

  return 0;
}

void cleanup_module(void)
{
  printk(KERN_INFO "Unloading eventfd");

  if(replacedSyscalls)
  {
    if(sys_call_tablePtr[__NR_eventfd] != mod_eventfd ||
       sys_call_tablePtr[__NR_eventfd2] != mod_eventfd2)
      printk(KERN_ALERT "Eventfd syscall has been tampered with elsewhere");

    sys_call_tablePtr[__NR_eventfd] = original_eventfd;
    sys_call_tablePtr[__NR_eventfd2] = original_eventfd2;

    pageTable[pindex] = old_page_table_entry;
  }
  else
  {
    printk(KERN_INFO "Not switching syscalls");
  }

  replacedSyscalls = false;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tryneus");
MODULE_DESCRIPTION("Work in progress - new flag in eventfd");

