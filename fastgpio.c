#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include "fastgpio.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Przemyslaw Borkowski");
MODULE_DESCRIPTION("My way to handle gpios from userspace via devfs, is much faster because we have one write fd for read and write");

int init_module(void)
{
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
	memset(&gpio_can_sleep_table,0,ARCH_NR_GPIOS);
	memset(&requested_gpios,0,ARCH_NR_GPIOS * sizeof(gpio_requested));
    return 0;
}

void cleanup_module(void)
{
  int i;
  // release gpios before unloading module
  for (i=0; i< ARCH_NR_GPIOS; i++)
  	if (requested_gpios[i].read || requested_gpio[i].write) gpio_free(i);
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
    int i,j;
    if (!gpio_read_num_set)
	{
	printk(KERN_DEBUG "fastgpio: no gpios to read are set use ioctl to set desired gpios");
	return 0;
	}
#ifdef DEBUG
	printk(KERN_DEBUG "fastgpio: size: %d gpio_num: %d",(int)length,gpio_read_num_set);
#endif
   if (length > gpio_read_num_set) j = gpio_read_num_set; else j = length;
	for (i = 0; i < j; i++)
		{
		gpio_read[i] = (gpio_can_sleep_table[i] == 1) ? gpio_get_value(gpio_read_ports[i]) : gpio_get_value_cansleep(gpio_read_ports[i]); 
#ifdef DEBUG
		printk(KERN_DEBUG "%d - port %d - value %d - gpio_funct %s",i,gpio_read_ports[i],gpio_read[i],(gpio_can_sleep_table[i] == 0) ? "normal" : "sleep" );
#endif
		}
    if (!copy_to_user(buffer,gpio_read,gpio_read_num_set)) return j;
	return 0;
}

static ssize_t gpr_device_write(struct file *filp, const char *buff, size_t len, loff_t * offset)
{
    int i;
    if (len > MAX_GPIO) 
	{
	printk(KERN_DEBUG "fastgpio: length - %d is greather than %d",len,MAX_GPIO);
	return 0;
	}
   if (len > gpio_read_num_set)
	{
	printk(KERN_DEBUG "fastgpio: length - %d is greather than actualy set pins %d",len,gpio_read_num_set);
	return 0;
	}
#ifdef DEBUG
   printk(KERN_DEBUG "fastgpio: write length - %d",len);
#endif
   i = copy_from_user(&gpio_write,buff,(int)len);
   for(i=0; i < len; i++)
	{
	(gpio_can_sleep_table[i] == 1) ? gpio_set_value(gpio_write_ports[i],gpio_write[i]) : gpio_set_value_cansleep(gpio_write_ports[i],gpio_write[i]);
#ifdef DEBUG
	printk(KERN_DEBUG "%d - set port %d - value %d",i,gpio_write_ports[i],(unsigned char)gpio_write[i]);
#endif
	}
   return len;
}


int gpr_request_gpio(unsigned pin,unsigned long type)
	{
	// we have 2 gpio types in and out, check present tables for that we requested them already, or request them
	int i = 0;
	if (gpio_is_valid(pin) != 1) return -EACCES;
#ifdef DEBUG
	printk(KERN_DEBUG "checking is already requested");
#endif
	if (!(requested_gpios[i].read || requested_gpios[i].write))
		i = gpio_request_one(pin,type,"");
	if (i) return -EACCES;
		
	if (type == GPIOF_OUT_INIT_LOW)
		requested_gpios[i].write = 1;
	if (type == GPIOF_IN)
		requested_gpios[i].read = 1;
	}

void release_unused_gpios(gpio_ioctl &gpios, unsigned long type)
	{
	int i = 0;
	// go through table, find what was released, and release it with gpio_free
	}
	
static long gpr_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
	{
	gpio_ioctl tmp;
	int i,j,k,pin;

	if (copy_from_user(&tmp, (gpio_ioctl *)arg,sizeof(gpio_ioctl)))
		return -EACCES;

	switch (cmd)
		{
		case FASTGPIO_SET_PINS:
			printk(KERN_DEBUG "fastgpio: checking that pins are usable");
			// check if the pins are present and useble on board, is direction out, or set direction to out
			for (i=0;i < tmp.number; i++)
				if (gpr_request_gpio(tmp.pins[i],GPIOF_OUT_INIT_LOW))
					return -EACCES;
// release unused gpios
			release_unused_gpios(&tmp,GPIOF_OUT_INIT_LOW);
/*
			for (i=0; i < gpio_write_num_set; i++)
				{
				k = 0;
				j = 0;
				pin = gpio_write[i];
				while ((j < tmp.number) && (k == 0))
					{
					if (tmp.pins[i] == pin) k = 1;
					j++;
					}
				while ((j < gpio_read_num_set) && (k == 0))
					{
					if (gpio_read_ports[i] == pin) k = 1;
					j++;
					}
				if (!k) 
					{
#ifdef DEBUG
					printk(KERN_DEBUG "fastgpio: releasing gpio number %d",pin);
#endif
					gpio_free(pin);
					}
				}
*/			
			gpio_write_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				{
				gpio_write_ports[i] = tmp.pins[i];
				printk(KERN_DEBUG "fastgpio: write %d", tmp.pins[i]);
				}
			printk(KERN_DEBUG "fastgpio: setting %d pins to read",tmp.number);
			break;
		case FASTGPIO_READ_PINS:
			printk(KERN_DEBUG "fastgpio: checking that pins are usable");
			// check if the pins are present and useble on board etc...

			gpio_read_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				{
				gpio_read_ports[i] = tmp.pins[i];
				printk(KERN_DEBUG "fastgpio: read %d", tmp.pins[i]);
				}
			printk(KERN_DEBUG "fastgpio: setting %d pins to write",tmp.number);
			break;
		case FASTGPIO_SET_DIR:
			printk(KERN_DEBUG "fastgpio: setting pins direction");
			for (i=0; i < tmp.number; i++)
				{
				(tmp.dir[i] == 1) ? gpio_direction_input(tmp.pins[i]) : gpio_direction_output(tmp.pins[i],0);
				printk(KERN_DEBUG "fastgpio: set direction of pin %d to %s",tmp.pins[i],(tmp.dir[i] == 1) ? "input" : "output");
				}
			break;
		}
	return 0;
	}
