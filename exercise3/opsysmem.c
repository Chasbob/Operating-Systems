#include <linux/init.h>   // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/device.h> // Header to support the kernel Driver Model
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/fs.h>     // Header for the Linux file system support
#include <asm/uaccess.h>  // Required for the copy to user function
#include <linux/mutex.h>  // Required for the mutex functionality
#include <linux/slab.h>
#include "opsysmem.h"
#include "ioctl.h"

MODULE_LICENSE("GPL"); ///< The license type -- this affects available functionality

DEFINE_MUTEX  (devLock);
static int counter = 0;

static long device_ioctl(struct file *file,	/* see include/linux/fs.h */
		 unsigned int ioctl_num,	/* number and param for ioctl */
		 unsigned long ioctl_param)
{

	/* 
	 * Switch according to the ioctl called 
	 */
	if (ioctl_num == RESET_COUNTER) {
	    counter = 0; 
	    /* 	    return 0; */
	    return 5; /* can pass integer as return value */
	}

	else {
	    /* no operation defined - return failure */
	    return -EINVAL;

	}
}
int opsysmem_init(void)
{
   printk(KERN_INFO "opsysmem: Initializing the opsysmem LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   Major = register_chrdev(0, DEVICE_NAME, &fops);
   if (Major < 0)
   {
      printk(KERN_ALERT "opsysmem failed to register a major number\n");
      return Major;
   }
   printk(KERN_INFO "opsysmem: I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "opsysmem: the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "opsysmem: Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "opsysmem: the device file.\n");
	printk(KERN_INFO "opsysmem: Remove the device file and module when done.\n");
   data = kmalloc(INITIAL_CAPACITY * sizeof(char **), GFP_KERNEL);
   mutex_init(&devLock); // Initialize the mutex dynamically
   return 0;
}

void opsysmem_exit(void)
{
   unregister_chrdev(Major, DEVICE_NAME);          // unregister the major number
   printk(KERN_INFO "opsysmem: Goodbye from the LKM!\n");
}

static int device_open(struct inode *inodep, struct file *filep)
{

   if (!mutex_trylock(&devLock))
   { // Try to acquire the mutex (returns 0 on fail)
      printk(KERN_ALERT "opsysmem: Device in use by another process");
      return -EBUSY;
   }
   // numberOpens++;
   // printk(KERN_INFO "opsysmem: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, len);

   if (error_count == 0)
   { // success!
      printk(KERN_INFO "opsysmem: Sent %d characters to the user\n", len);
      return (len = 0); // clear the position to the start and return 0
   }
   else
   {
      printk(KERN_INFO "opsysmem: Failed to send %d characters to the user\n", error_count);
      return -EFAULT; // Failed -- return a bad address message (i.e. -14)
   }
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   strcat(message, buffer);
   if (len > MSG_MAX_SIZE)
   {
   }
   // if (size_of_messages + len > LIST_MAX_SIZE)
   // {
   // }
   // sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   // size_of_message = strlen(message); // store the length of the stored message
   printk(KERN_INFO "opsysmem: Received %zu characters from the user: (%s)\n", len, buffer);
   printk(KERN_INFO "opsysmem: message = (%s)\n", message);
   return len;
}

static int device_release(struct inode *inodep, struct file *filep)
{
   mutex_unlock(&devLock); // release the mutex (i.e., lock goes up)
   kfree(data);
   printk(KERN_INFO "opsysmem: Device successfully closed\n");
   return 0;
}