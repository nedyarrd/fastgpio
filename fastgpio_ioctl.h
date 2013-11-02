#include "fastgpio.h"
typedef struct
	{
	int number;
	int pins[MAX_GPIO];
	} gpio_ioctl;
// which pins We will set
#define FASTGPIO_SET_PINS _IOW('f',1, gpio_ioctl *)
// which pins We will read
#define FASTGPIO_READ_PINS _IOW('f',2, gpio_ioctl *)