#include <linux/module.h>	//Always needed for kernels.
#include <linux/kernel.h>	//Needed for KERN_INFO
#include <linux/init.h>		//Gives us __init & __exit
#include <linux/fs.h>		//Gives us file_operations
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <asm/uaccess.h>		//Gives us put_user

#define DRIVER_AUTHOR "Jacob [Cassagnol & Crandal]"
#define DRIVER_DESC   "Assignment 5 character device driver"
#define DEVICE_NAME "char_driver"
#define CLASS_NAME "char_class"

#define MAX_MEMORY 2048

/// @brief Removes taint message, declares author & description.
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

// Prototypes.
// 'static' keeps everything internally linked.
static int char_driver_init(void);
static ssize_t char_driver_read(struct file *, char *, size_t, loff_t *);
static ssize_t char_driver_write(struct file *, const char *, size_t, loff_t *);
static int char_driver_open(struct inode *, struct file *);
static int char_driver_release(struct inode *, struct file *);

static int insert_queue(const char* vals, int vals_len);
static int remove_queue(char* vals, int vals_len);

// Our queue, statically allocated.
static char queue[MAX_MEMORY] = {0};
static int  front = 0;
static int  back = 0;
// Keep track of number of times device is open to allow for correct cleanup.
static int deviceUser = 0;
//Stores the automatic device number
static int majorNumber;  
static struct class* charClass = NULL;
static struct device* charDevice = NULL;

/// @brief file_operations links our functions to the system calls.
static struct file_operations fops = {
	.read = char_driver_read,
	.write = char_driver_write,
	.open = char_driver_open,
	.release = char_driver_release,
};

/// @brief Initialization component for the character driver.
static int __init char_driver_init (void) {
	printk(KERN_INFO "Installing module.\n");
	

	majorNumber = register_chrdev(0,DEVICE_NAME, &fops);
        if(majorNumber < 0){
                printk(KERN_ALERT "%s : Failed to register a major number\n",DEVICE_NAME);
                return majorNumber;
        }
	
	charClass = class_create(THIS_MODULE, CLASS_NAME);
	//TODO: Error checking here
	charDevice = device_create(charClass,NULL, MKDEV(majorNumber,0),NULL,DEVICE_NAME);
	//TODO:Error checking here

        printk(KERN_INFO "%s : Installed correctly with major number : %d\n",DEVICE_NAME, majorNumber);
	return 0;
}

/// @brief When the character driver is unloaded, this gets called.
static void __exit char_driver_exit (void) {
	printk(KERN_INFO "Removing module.\n");
	
	device_destroy(charClass,MKDEV(majorNumber,0));
	class_unregister(charClass);
	class_destroy(charClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
}


/// @brief   Safely inserts items from vals into the queue.
/// @returns Returns count of bytes read into queue.
static int insert_queue(const char* vals, int vals_len)
{
	int bytes_read = 0;

	for ( ; bytes_read < vals_len; bytes_read++)
	{
		if ((back + 1) % MAX_MEMORY == front)
			break;

		queue[back++] = vals[bytes_read];

		if (back >= MAX_MEMORY)
			back = 0;
	}

	return bytes_read;
}

/// @brief Safely removes items from queue and places them in vals.
/// @returns Returns count of bytes removed from queue, or negative value for error.
static int remove_queue(char* vals, int vals_len)
{
	int bytes_read = 0, errorVal = 0;
	for ( ; bytes_read < vals_len; bytes_read++)
	{
		if (front == back)
			break;

		// Error condtion is non-zero return value.
		errorVal = put_user(queue[front], &vals[bytes_read]);
		if (errorVal != 0)
			return errorVal > 0 ? -errorVal : errorVal;

		front++;
		if (front >= MAX_MEMORY)
			front = 0;
	}

	return bytes_read;
}

/// @brief
static ssize_t char_driver_read(struct file * filp, char * bufIn, size_t lenIn, loff_t * offset)
{
	printk(KERN_INFO "Reading char driver.\n");

	return remove_queue(bufIn,lenIn);
}

/// @brief used when device is written to.
static ssize_t char_driver_write(struct file * filp, const char * bufOut, size_t lenOut, loff_t * offset)
{
	printk(KERN_INFO "Writing char driver.\n");

	return insert_queue(bufOut,lenOut);
}

/// @brief used when device is opened.
static int char_driver_open(struct inode * inNode, struct file * filp)
{
	printk(KERN_INFO "Opening char driver.\n");
	if (deviceUser == 1)
	{
		printk(KERN_INFO "ERROR: Already have an open connection with char driver. FAILED.\n");
		return -EBUSY;
	}
	deviceUser = 1;
	//increments the usage count if possible;
	try_module_get(THIS_MODULE);
	return 0;
}

/// @brief used when device is closed.
static int char_driver_release(struct inode * inNode, struct file * filp)
{
	printk(KERN_INFO "Releasing char driver.\n");
	deviceUser = 0;
	//decrements the usage count if possible;
	module_put(THIS_MODULE);
	return 0;
}

module_init(char_driver_init);
module_exit(char_driver_exit);
