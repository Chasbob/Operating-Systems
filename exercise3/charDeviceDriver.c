
#include <linux/init.h>   // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/device.h> // Header to support the kernel Driver Model
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/fs.h>	 // Header for the Linux file system support
#include <asm/uaccess.h>  // Required for the copy to user function
#include <linux/mutex.h>  // Required for the mutex functionality
#include <linux/slab.h>
#include "ioctl.h"
// #include "queue.h"
#include "charDeviceDriver.h"
MODULE_LICENSE("GPL");										  ///< The license type -- this affects available functionality
// MODULE_AUTHOR("Derek Molloy");								  ///< The author -- visible when you use modinfo
// MODULE_DESCRIPTION("A simple Linux char driver for the BBB"); ///< The description -- see modinfo
// MODULE_VERSION("0.1");										  ///< A version number to inform users


static int length = 0;
static int bytes_read = 0;
// static char* msg_ptr = 0;
struct QNode
{
	char *key;
	struct QNode *next;
	int length;
};
struct Queue
{
	struct QNode *front, *rear;
	int size;
};
struct QNode *newNode(char *k, int length)
{
	struct QNode *temp = kmalloc(sizeof(struct QNode), GFP_KERNEL);
	// char *ptr;
	size_t i;
	if (temp == NULL)
	{
		printk(KERN_INFO "charDeviceDriver - newNode: kmalloc returned NULL\n");
		return NULL;
	}
	temp->key = kmalloc(length, GFP_KERNEL);
	// ptr = kmalloc(length, GFP_KERNEL);
	if(temp->key == NULL){
		printk(KERN_INFO "charDeviceDriver - newNode->key: kmalloc returned NULL\n");
		return NULL;
	}
	for ( i = 0; i < length; i++)
	{
		*(temp->key + i)= *(k + i);
	}
	// strcpy((char *)temp->key, ptr);
	// strcat((char*)temp->key, "\0");
	temp->length = length;
	temp->next = NULL;
	printk(KERN_INFO "charDeviceDriver - newNode{key: %.*s}", temp->length, k);
	// kfree(ptr);
	return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue(void)
{
	struct Queue *q;
	q = kmalloc(sizeof(struct Queue), GFP_KERNEL);
	if (q == NULL)
	{
		printk(KERN_INFO "charDeviceDriver - createQueue: kmalloc returned NULL\n");
		return NULL;
	}
	q->front = q->rear = NULL;
	q->size = 0;

	return q;
}

// The function to add a key k to q
void enQueue(struct Queue *q, char *k, int length)
{
	struct QNode *temp;
	temp = newNode(k, length);
	if (temp == NULL)
	{
		printk(KERN_INFO "charDeviceDriver - enQueue: newNode returned NULL");
		return;
	}
	printk(KERN_INFO "charDeviceDriver - enQueue: %s,%i", temp->key, q->size);
	// If queue is empty, then new node is front and rear both
	if (q->rear == NULL)
	{
		q->front = q->rear = temp;
		q->size++;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
	q->size++;
}

// Function to remove a key from given queue q
void deQueue(struct Queue *q)
{
	struct QNode *temp;
	// If queue is empty, return NULL.
	if (q == NULL)
	{
		printk(KERN_INFO "charDeviceDriver - deQueue: q == NULL\n");
		return;
	}
	if (q->front == NULL)
	{
		printk(KERN_INFO "charDeviceDriver - deQueue: q->front == NULL\n");
		return;
	}
	// Store previous front and move front one node ahead
	temp = q->front;
	kfree(temp);

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;
	q->size = q->size - 1;
}

struct QNode *peek(struct Queue *q)
{
	if (q != NULL && q->front != NULL)
	{
		printk(KERN_INFO "charDeviceDriver - peek: returning key->%s\n", q->front->key);
		return q->front;
	}
	else
	{
		return NULL;
	}
}

char *peekTail(struct Queue *q)
{
	if (q != NULL && q->rear != NULL)
	{
		return q->rear->key;
	}
	else
	{
		return NULL;
	}
}

static DEFINE_MUTEX(dev_mutex); ///< Macro to declare a new mutex

static long device_ioctl(struct file *file,		 /* see include/linux/fs.h */
						 unsigned int ioctl_num, /* number and param for ioctl */
						 unsigned long ioctl_param)
{
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

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	messages = createQueue();
	if (messages == NULL)
	{
		printk(KERN_INFO "charDeviceDriver: createQueue returned NULL to init, failing...\n");
		cleanup_module();
		return 0;
	}
	printk(KERN_INFO "charDeviceDriver: createQueue returned %u to init\n", sizeof(messages));
	mutex_init(&dev_mutex); // Initialize the mutex dynamically
	return SUCCESS;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME); // unregister the major number
	// TODO free the queue
	printk(KERN_INFO "charDeviceDriver: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int device_open(struct inode *inodep, struct file *filep)
{
	mutex_lock(&dev_mutex);
	if (numberOpens)
	{
		mutex_unlock(&dev_mutex);
		return -EBUSY;
	}
	numberOpens++;
	mutex_unlock(&dev_mutex);
	printk(KERN_INFO "charDeviceDriver: Device has been opened %d time(s)\n", numberOpens);
	try_module_get(THIS_MODULE);
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
static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int result;
	struct QNode *temp;
	char *str;
	printk(KERN_INFO "charDeviceDriver: device_read of length (%i)", len);
	temp = peek(messages);
	if (temp == NULL)
	{
		printk(KERN_INFO "charDeviceDriver: read - NULL, no messages\n");
		return -EAGAIN;
	}
	length = temp->length;
	str = kmalloc(strlen(temp->key), GFP_KERNEL);
	strcpy(str,temp->key);
	while (length && *str) {
		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		printk(KERN_INFO "charDeviceDriver: put -> (%c)",*str);
		result = put_user(*(str++), buffer++);
		if (result != 0) {
			return -EFAULT;
		}
		length--;
		bytes_read++;
	}
	deQueue(messages);
	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	printk(KERN_INFO "charDeviceDriver: bytes_read (%i)\n",bytes_read);
	return bytes_read;

}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using message[x] = buffer[x]
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{	
	printk(KERN_INFO "charDeviceDriver: write: len -> (%i): (%.*s)\n", len, len, buffer);
	if (len > MSG_MAX_SIZE)
	{
		printk(KERN_INFO "charDeviceDriver: message must be smaller than %i\n", MSG_MAX_SIZE);
		return len;
	}
	if (total_size + len > LIST_MAX_SIZE)
	{
		printk(KERN_INFO "charDeviceDriver: This op would exced max size of %i\n", MSG_MAX_SIZE);
		return len;
	}
	// printk(KERN_INFO "charDeviceDriver: buffer -> (%s)", buffer);
	enQueue(messages, (char*)buffer, len);
	total_size = total_size + len;
	// printk(KERN_INFO "charDeviceDriver: total_size -> (%i)", total_size);

	printk(KERN_INFO "charDeviceDriver: %i messages, Received (%s) from the user\n", messages->size, peekTail(messages));
	return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int device_release(struct inode *inodep, struct file *filep)
{
	mutex_lock(&dev_mutex);
	numberOpens--; /* We're now ready for our next caller */
	mutex_unlock(&dev_mutex); // release the mutex (i.e., lock goes up)
	module_put(THIS_MODULE);
	printk(KERN_INFO "charDeviceDriver: Device successfully closed\n");
	return 0;
}
