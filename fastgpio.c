#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include "fastgpio.h"

#define DEBUG 1
//#define MAX_GPIO 10

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Przemyslaw Borkowski");
MODULE_DESCRIPTION("My way to handle gpios from userspace via devfs, is much faster because we have one write fd for read and write");

int init_module(void)
{
	printk(KERN_DEBUG "FASTGPIO INIT");
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
	memset(&gpio_can_sleep_table,0,MAX_GPIO);
	memset(&requested_gpios,0,MAX_GPIO * sizeof(gpio_requested));
    return 0;
}

void cleanup_module(void)
{
  int i;
  // release gpios before unloading module
  for (i=0; i< MAX_GPIO; i++)
	{
	printk(KERN_DEBUG "fastgpio: cleanup i = %d, read = %d, write = %d",i,(requested_gpios[i].read == 1) ? 1 : 0,(requested_gpios[i].write == 1) ? 1 : 0);
	
  	if ((requested_gpios[i].read == 1) || (requested_gpios[i].write == 1)) 
		{
#ifdef DEBUG
		printk(KERN_DEBUG "fastgpio: cleanup releasing %d",i);
#endif
		gpio_free(i);
		}
	}
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
#ifdef DEBUG2
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
   i = copy_from_user(&gpio_write,buff,(int)len);
   for(i=0; i < len; i++)
	(gpio_can_sleep_table[i] == 1) ? gpio_set_value(gpio_write_ports[i],gpio_write[i]) : gpio_set_value_cansleep(gpio_write_ports[i],gpio_write[i]);
   return len;
}


int gpr_request_gpio(unsigned pin,unsigned long type)
	{
	// check if the pins are present and useble on board, is direction out, or set direction to out
	// we have 2 gpio types in and out, check present tables for that we requested them already, or request them
	int i = 0;
	if (gpio_is_valid(pin) != 1) return -EACCES;
	if (!((requested_gpios[pin].read == 1) || (requested_gpios[pin].write == 1)))
		i = gpio_request_one(pin,type,"");
	if (i) return -EACCES; else return 0;
	}

#define REL_UNUSED(tbl_change,tbl_stay) for (i=0; i < MAX_GPIO; i++) { \
		if ((requested_gpios[i].read || requested_gpios[i].write) && (tbl_stay || rq_tmp[i])) \
			gpio_free(i); \
		tbl_change = rq_tmp[i]; }

static long gpr_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
	{
	gpio_ioctl tmp;
	int i;
	unsigned char rq_tmp[MAX_GPIO];

	if (copy_from_user(&tmp, (gpio_ioctl *)arg,sizeof(gpio_ioctl)))
		return -EACCES;

	switch (cmd)
		{
		case FASTGPIO_SET_PINS:
			// clear rq_tmp to see what was set
			memset(&rq_tmp,0,MAX_GPIO*sizeof(unsigned char));
			for (i=0;i < tmp.number; i++)
				if (gpr_request_gpio(tmp.pins[i],GPIOF_OUT_INIT_LOW))
					return -EACCES;
					else 
					rq_tmp[tmp.pins[i]] = 1;
			REL_UNUSED(requested_gpios[i].write,requested_gpios[i].read);
			gpio_write_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				gpio_write_ports[i] = tmp.pins[i];
			break;
		case FASTGPIO_READ_PINS:
			memset(&rq_tmp,MAX_GPIO, sizeof(unsigned char));
			for (i=0;i < tmp.number; i++)
				if (gpr_request_gpio(tmp.pins[i],GPIOF_IN))
					return -EACCES;
					else 
					rq_tmp[i] = 1;
			REL_UNUSED(requested_gpios[i].read,requested_gpios[i].write);
			gpio_read_num_set = tmp.number;
			for (i=0;i < tmp.number; i++)
				gpio_read_ports[i] = tmp.pins[i];
			break;
		case FASTGPIO_SET_DIR:
			// request gpio if necessary
			for (i=0; i < tmp.number; i++)
				(tmp.dir[i] == 1) ? gpio_direction_input(tmp.pins[i]) : gpio_direction_output(tmp.pins[i],0);
			break;
		}
	return 0;
	}
