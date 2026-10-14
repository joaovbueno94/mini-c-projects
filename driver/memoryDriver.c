// Copyright 2026 João Vinícius Bueno

#include <linux/fs.h> // file_operations structure
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h> // Modern registration API
#include <linux/module.h>     // pr_info(), pr_err()
#include <linux/mutex.h>      // Standard for handling concurrency
#include <linux/uaccess.h>    // copy_to_user/copy_from_user

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("João Vinícius Bueno");
MODULE_DESCRIPTION("Modern 64-bit Memory Driver");

// Storage for 64-bit value
static u64 memory_value = 0;
// Lock to prevent race conditions
static DEFINE_MUTEX(memory_lock);

/**
 * memory_read - Transfers data from Kernel to User Space
 * Equivalent to 'fread' or 'cat' in User Space.
 */
static ssize_t memory_read(struct file *filp, char __user *buf, size_t count,
                           loff_t *f_pos) {
  // If the user has already read the value, return 0 (End of File)
  if (*f_pos > 0) {
    return 0;
  }

  mutex_lock(&memory_lock);

  // Standard safe transfer to User Space buffer
  if (copy_to_user(buf, &memory_value, sizeof(u64))) {
    mutex_unlock(&memory_lock);
    return -EFAULT;
  }

  mutex_unlock(&memory_lock);

  *f_pos += sizeof(u64);

  // Return 8 bytes read
  return sizeof(u64);
}

/**
 * memory_write - Transfers data from User Space to Kernel Space
 * Equivalent to 'fwrite' or 'echo' in User Space.
 */
static ssize_t memory_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *f_pos) {
  mutex_lock(&memory_lock);

  // Standard safe transfer from User Space buffer
  if (copy_from_user(&memory_value, buf, sizeof(u64))) {
    mutex_unlock(&memory_lock);
    return -EFAULT;
  }

  mutex_unlock(&memory_lock);

  // Return 8 bytes written
  return sizeof(u64);
}

// Link standard file operations to our functions
static const struct file_operations memory_fops = {
    .owner = THIS_MODULE,
    // Link to our read function
    .read = memory_read,
    // Link to our write function
    .write = memory_write,
};

// Modern registration structure
static struct miscdevice memory_dev = {
    // Kernel assigns a free minor number
    .minor = MISC_DYNAMIC_MINOR,
    // Creates /dev/memory automatically
    .name = "memory",
    .fops = &memory_fops,
};

// The inicialization function
static int __init memory_init(void) {
  // Register driver
  int ret = misc_register(&memory_dev);

  if (ret) {
    pr_err("memory: Failed to register device\n");
    return ret;
  }

  // pr_info works similarly to printf but at the kernel level
  pr_info("memory: Modern 64-bit driver loaded\n");

  return 0;
}

// the exit function
static void __exit memory_exit(void) {
  // Unregister and clean up
  misc_deregister(&memory_dev);
  pr_info("memory: Driver unloaded\n");
}

// Macros to register the entry and exit points with the kernel
module_init(memory_init);
module_exit(memory_exit);
