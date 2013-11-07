#define MAX_GPIO 128
typedef struct
	{
	int number;
	int pins[MAX_GPIO]; // pin number to set what to read or write
	int dir[MAX_GPIO]; // coresponding direction of pin - 0 output; 1 input
	} gpio_ioctl;
// which pins We will set
// caution it defaults that pins are set to out direction
#define FASTGPIO_SET_PINS _IOW('f',1, gpio_ioctl *)
// which pins We will read
// caution this don't set direction of pins, because we can read state of in or out pins
#define FASTGPIO_READ_PINS _IOW('f',2, gpio_ioctl *)
// setting direction for group of pins
// caution when direction set to out its value is set to 0
#define FASTGPIO_SET_DIR _IOW('f',3,gpio_ioctl *)
