#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>/* for put_user */
#include "fastgpio.h"

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
    int i,j,k;
	long off;
	off = (long)*offset;
    if (!gpio_read_num_set)
	{
	printk(KERN_DEBUG "fastgpio: no gpios to read are set use ioctl to set desired gpios");
	return 0;
	}
    if (off > gpio_read_num_set)
	return 0;
	
	printk(KERN_DEBUG "fastgpio: offset: %ld gpio_num: %d", off,gpio_read_num_set);
   if ((off+length) > (gpio_read_num_set)) j = gpio_read_num_set; else j = off+length;
	k = 0;
	for (i = off; i < j; i++)
		{
		gpio_read[i] = gpio_can_sleep_table[i] ? gpio_get_value(gpio_read_ports[i]) : gpio_get_value_cansleep(gpio_read_ports[i]); ;
		k++;
		}
    if (!copy_to_user(buffer,&gpio_read+(long)*offset,gpio_read_num_set-(long)*offset)) return gpio_read_num_set - (long)*offset;
	return k;
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
	gpio_ioctl tmp;
	int i;
	switch (cmd)
		{
		case FASTGPIO_SET_PINS:
			if (copy_from_user(&tmp, (gpio_ioctl *)arg,sizeof(gpio_ioctl)))
				return -EACCES;
			printk(KERN_DEBUG "fastgpio: checking that pins are usable");
			// check if the pins are present and useble on board etc...

			gpio_write_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				{
				gpio_write_ports[i] = tmp.pins[i];
				printk(KERN_DEBUG "fastgpio: write %d", tmp.pins[i]);
				}
			printk(KERN_DEBUG "fastgpio: setting %d pins to read",tmp.number);
			return 0;
			break;
		case FASTGPIO_READ_PINS:
			if (copy_from_user(&tmp, (gpio_ioctl *)arg,sizeof(gpio_ioctl)))
				return -EACCES;
			printk(KERN_DEBUG "fastgpio: checking that pins are usable");
			// check if the pins are present and useble on board etc...

			gpio_read_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				{
				gpio_read_ports[i] = tmp.pins[i];
				printk(KERN_DEBUG "fastgpio: read %d", tmp.pins[i]);
				}
			printk(KERN_DEBUG "fastgpio: setting %d pins to write",tmp.number);
			return 0;
			break;
		}
	return 0;
	}
