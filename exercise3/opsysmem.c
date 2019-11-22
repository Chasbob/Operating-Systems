
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#include <linux/mutex.h>	  // Required for the mutex functionality
#include<linux/slab.h>
#include "opsysmem.h"
#include "ioctl.h"

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Derek Molloy");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the BBB");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users


static DEFINE_MUTEX(opsysmem_mutex);	    ///< Macro to declare a new mutex

static long device_ioctl(struct file *file,		 /* see include/linux/fs.h */
						 unsigned int ioctl_num, /* number and param for ioctl */
						 unsigned long ioctl_param){
	/* 
	 * Switch according to the ioctl called 
	 */
	if (ioctl_num == RESET_COUNTER)
	{
		counter = 0;
		/* 	    return 0; */
		return 5; /* can pass integer as return value */
	}

	else
	{
		/* no operation defined - return failure */
		return -EINVAL;
	}
}

int init_module(void){
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
   messages = kmalloc(sizeof(char*), GFP_KERNEL);
   mutex_init(&opsysmem_mutex);          // Initialize the mutex dynamically
	return SUCCESS;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
void cleanup_module(void){
   unregister_chrdev(Major, DEVICE_NAME);         // unregister the major number
   kfree(messages);
   printk(KERN_INFO "opsysmem: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int device_open(struct inode *inodep, struct file *filep){

   if(!mutex_trylock(&opsysmem_mutex)){                  // Try to acquire the mutex (returns 0 on fail)
	printk(KERN_ALERT "OpSysMEM: Device in use by another process");
	return -EBUSY;
   }
   numberOpens++;
   printk(KERN_INFO "OpSysMEM: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){           // success!
      printk(KERN_INFO "OpSysMEM: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0); // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "OpSysMEM: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;      // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using message[x] = buffer[x]
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   if(len > MSG_MAX_SIZE){
      printk(KERN_INFO "OpSysMEM: message must be smaller than %i\n",MSG_MAX_SIZE);
      return len;
   }
   if(total_size + len > LIST_MAX_SIZE){
      printk(KERN_INFO "OpSysMEM: This op would exced max size of %i\n",MSG_MAX_SIZE);
      return len;
   }
   krealloc(messages, sizeof(messages) + len, GFP_KERNEL);
   total_size = sizeof(messages);
   // messages[message_count++] = buffer;
   sprintf(messages[message_count++],"%s",buffer);


   // sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   // size_of_message = strlen(len);                 // store the length of the stored message
   printk(KERN_INFO "OpSysMEM: %i messages, Received (%s) from the user\n", message_count,messages[message_count-1]);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int device_release(struct inode *inodep, struct file *filep){
   mutex_unlock(&opsysmem_mutex);                      // release the mutex (i.e., lock goes up)
   printk(KERN_INFO "OpSysMEM: Device successfully closed\n");
   return 0;
}
