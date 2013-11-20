#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "fastgpio_ioctl.h"

#ifndef FASTGPIO_H
#define FASTGPIO_H
int init_module(void);
void cleanup_module(void);
static int gpr_device_open(struct inode *, struct file *);
static int gpr_device_release(struct inode *, struct file *);
static ssize_t gpr_device_read(struct file *, char *, size_t, loff_t *);
static ssize_t gpr_device_write(struct file *, const char *, size_t, loff_t *);
static long gpr_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#define SUCCESS 0


static struct class *cl;

typedef struct {
        unsigned int read;
        unsigned int write;
      } gpio_requested;


static gpio_requested requested_gpios[ARCH_NR_GPIOS];
unsigned char gpio_read[ARCH_NR_GPIOS];
unsigned char gpio_write[ARCH_NR_GPIOS];
unsigned char  gpio_can_sleep_table[ARCH_NR_GPIOS];
int gpio_read_ports[ARCH_NR_GPIOS]; // -1 means not set 0-x x+1 = -1 
int gpio_write_ports[ARCH_NR_GPIOS];
int gpio_read_num_set;
int gpio_write_num_set;


static dev_t Major_read;/* Major number assigned to our device driver */
static struct cdev c_dev;
static int Device_Open = 0;

static struct file_operations fops_read = {
.read = gpr_device_read,
.write = gpr_device_write,
.open = gpr_device_open,
.release = gpr_device_release,
.unlocked_ioctl = gpr_device_ioctl
};

#endif
