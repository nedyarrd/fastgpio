/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>/* for put_user */
#include "fastgpio.h"
#include "fastgpio_ioctl.h"

MODULE_LICENSE("GPL");


int init_module(void)
{
//    Major_read = register_chrdev(0, DEVICE_NAME_READ, &fops_read);
  if (alloc_chrdev_region(&Major_read, 0, 1, "fastgpio") < 0)
	  {
	    return -1;
	  }
    if ((cl = class_create(THIS_MODULE, "fastgpio")) == NULL)
	  {
	    unregister_chrdev_region(Major_read, 1);
	    return -1;
	  }
   if (device_create(cl, NULL, Major_read, NULL, "fastgpio") == NULL)
	  {
	    class_destroy(cl);
	    unregister_chrdev_region(Major_read, 1);
	    return -1;
	  }
    cdev_init(&c_dev, &fops_read);
    if (cdev_add(&c_dev, Major_read, 1) == -1)
	  {
	    device_destroy(cl, Major_read);
	    class_destroy(cl);
	    unregister_chrdev_region(Major_read, 1);
	    return -1;
	  }
    return 0;
}

void cleanup_module(void)
{
  cdev_del(&c_dev);
  device_destroy(cl, Major_read);
  class_destroy(cl);
  unregister_chrdev_region(Major_read, 1);


}


static int gpr_device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
	return -EBUSY;
    Device_Open++;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int gpr_device_release(struct inode *inode, struct file *file)
{
    Device_Open--;
    module_put(THIS_MODULE);
    return 0;
}


static ssize_t gpr_device_read(struct file *filp,	/* see include/linux/fs.h   */
	       char *buffer,	/* buffer to fill with data */
	       size_t length,	/* length of the buffer     */
	       loff_t * offset)
{
//    int i;
	long off;
	off = (long)*offset;
    if (!gpio_read_num_set)
	{
	printk(KERN_DEBUG "fastgpio: no gpios to read are set use ioctl to set desired gpios");
	}
    if ((int)offset > gpio_read_num_set)
	return 0;
	
	printk(KERN_DEBUG "offset: %ld gpio_num: %d", (long)*offset,gpio_read_num_set);
//    for (i = (long)*offset; i <= gpio_read_num_set - (int)offset; i++)
//	gpio_read[i] = gpio_can_sleep_table[i] ? gpio_get_value(gpio_read_ports[i]) : gpio_get_value_cansleep(gpio_read_ports[i]); ;
//    if (!copy_to_user(buffer,&gpio_read+(long)*offset,gpio_read_num_set-(long)*offset)) return gpio_read_num_set - (long)*offset;
    return 0;
}

static ssize_t gpr_device_write(struct file *filp, const char *buff, size_t len, loff_t * offset)
{
    char *tmp[MAX_GPIO];
    if ((int)offset > gpio_read_num_set)
	return 0;
    return copy_from_user(tmp,buff,(int)len);
}


static long gpr_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
	{
	switch (cmd)
		{
		case FASTGPIO_SET_PINS:
			break;
		case FASTGPIO_READ_PINS:
			break;
		}
	}
