#include <linux/module.h>	//Always needed for kernels.
#include <linux/kernel.h>	//Needed for KERN_INFO
#include <linux/init.h>		//Gives us __init & __exit
#define DRIVER_AUTHOR "Jacob [Cassagnol & Crandal]"
#define DRIVER_DESC   "Assignment 5 character device driver"

#define MAX_MEMORY 2048

/// @brief Removes taint message, declares author & description.
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

/// @brief Can't use free - that's in cstdlib.
static char queue[MAX_MEMORY] = {0};
static int  front = 0;
static int  back = 0;

// static keeps everything inside this file.
static int __init char_driver_init (void) {
	printk(KERN_INFO "Installing module.\n");
	queue[0] = 'a';
	printk(KERN_INFO "Using queue, front, and back to remove error.\n%c %d %d\n", queue[0], front, back);
	return 0;
}

static void __exit char_driver_exit (void) {
	printk(KERN_INFO "Removing module.\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);
