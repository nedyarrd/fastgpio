#define MAX_GPIO 128
typedef struct
	{
	int number;
	int pins[MAX_GPIO];
	int dir[MAX_GPIO];
	} gpio_ioctl;
// which pins We will set
// caution it defaults that pins are set to out direction
#define FASTGPIO_SET_PINS _IOW('f',1, gpio_ioctl *)
// which pins We will read
// caution this don't set direction of pins, because we can read state of in or out pins
#define FASTGPIO_READ_PINS _IOW('f',2, gpio_ioctl *)
#define FASTGPIO_SET_DIR _IOW('f',3,gpio_ioctl *)
