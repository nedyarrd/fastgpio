#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "fastgpio_ioctl.h"

int main(void)
	{
	char *file_name = "/dev/fastgpio";
	int fd;
	gpio_ioctl gpio_sw;
	gpio_sw.number = 3;
	memset(&gpio_sw.pins,MAX_GPIO,sizeof(gpio_sw.pins));
	gpio_sw.pins[0] = 4;
	gpio_sw.pins[1] = 3;
	gpio_sw.pins[2] = 5;
	fd = open(file_name, O_RDWR);
	if (fd == -1)
		{
		perror("open of dev/fastgpio failed\n");
		return 1;
		}
	printf("dev/fastgpio ioctl SET_PINS\n");
	if (ioctl(fd,FASTGPIO_SET_PINS,&gpio_sw))
		{
		perror("ioctl SET_PINS failed\n");
		return 2;
		}
	printf("dev/fastgpio opened\n");
	if (ioctl(fd,FASTGPIO_READ_PINS,&gpio_sw))
		{
		perror("ioctl READ_PINS failed\n");
		return 3;
		}
	printf("dev/fastgpio ioctl READ_PINS\n");

	return 0;
	}