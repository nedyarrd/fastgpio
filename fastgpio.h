#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#ifndef FASTGPIO_H
#define FASTGPIO_H
int init_module(void);
void cleanup_module(void);
static int gpr_device_open(struct inode *, struct file *);
static int gpr_device_release(struct inode *, struct file *);
static ssize_t gpr_device_read(struct file *, char *, size_t, loff_t *);
static ssize_t gpr_device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME_READ	"gpioread"/* Dev name as it appears in /proc/devices   */
#define DEVICE_NAME_WRITE	"gpiowrite"/* Dev name as it appears in /proc/devices   */
#define MAX_GPIO 128
#define BUF_LEN		MAX_GPIO /* Max length of the message from the device */
#define BUF_LEN_SET	MAX_GPIO*4 // I assume that gpio number is 3 digits + space or colon or something


static struct class *cl;


unsigned char gpio_read[MAX_GPIO];
unsigned char gpio_write[MAX_GPIO];
unsigned int  gpio_can_sleep_table[MAX_GPIO];
int gpio_read_ports[MAX_GPIO]; // -1 means not set 0-x x+1 = -1 
int gpio_write_ports[MAX_GPIO];
int gpio_read_num_set;
int gpio_write_num_set;


static dev_t Major_read;/* Major number assigned to our device driver */
static struct cdev c_dev;
static int Device_Open = 0;

static struct file_operations fops_read = {
.read = gpr_device_read,
.write = gpr_device_write,
.open = gpr_device_open,
.release = gpr_device_release
};

#endif