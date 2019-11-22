// /* Global definition for the example character device driver */

// int init_module(void);
// void cleanup_module(void);
// static int device_open(struct inode *, struct file *);
// static int device_release(struct inode *, struct file *);
// static ssize_t device_read(struct file *, char *, size_t, loff_t *);
// static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
// static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long);

// #define SUCCESS 		   0
// #define BUF_LEN 		   80		/* Max length of the message from the device */
// #define DEVICE_NAME        "opsysmem"    ///< The device will appear at /dev/opsysmem using this value
// #define LIST_MAX_SIZE      4194304
// #define MSG_MAX_SIZE       4096
// #define INITIAL_CAPACITY   1


// static int    majorNumber;                  ///< Store the device number -- determined automatically
// static char   message[MSG_MAX_SIZE] = {0};           ///< Memory for the string that is passed from userspace
// static short  size_of_message;              ///< Used to remember the size of the string stored
// static short  size_of_messages;              ///< Used to remember the size of the string stored
// static int    numberOpens = 0;              ///< Counts the number of times the device is opened

// /* 
//  * Global variables are declared as static, so are global within the file. 
//  */
// struct cdev *my_cdev;
//        dev_t dev_num;

// static int Major;		/* Major number assigned to our device driver */
// static int Device_Open = 0;	/* Is device open?  
// 				 * Used to prevent multiple access to device */
// static char msg[BUF_LEN];	/* The msg the device will give when asked */
// static char *msg_Ptr;



// static struct file_operations fops = {
// 	.read = device_read,
// 	.write = device_write,
// 	.open = device_open,
// 	.unlocked_ioctl = device_ioctl,
// 	.release = device_release
// };




/* Global definition for the example character device driver */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define LIST_MAX_SIZE      4194304
#define MSG_MAX_SIZE       4096
#define INITIAL_CAPACITY   1

/* 
 * Global variables are declared as static, so are global within the file. 
 */
struct cdev *my_cdev;
       dev_t dev_num;

static int Major;		/* Major number assigned to our device driver */
// static int Device_Open = 0;	/* Is device open?  
				//  * Used to prevent multiple access to device */
static char message[BUF_LEN];	/* The msg the device will give when asked */
static char** data;
// static char *msg_Ptr;



static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.unlocked_ioctl = device_ioctl,
	.release = device_release
};

