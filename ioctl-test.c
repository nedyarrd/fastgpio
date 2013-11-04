#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "fastgpio_ioctl.h"


#define MAX_NUM 3
int main(void)
	{
	char *file_name = "/dev/fastgpio";
	char *tmp[10];
	int fd,i;
	gpio_ioctl gpio_sw;
	gpio_sw.number = MAX_NUM;
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
	printf("testing file reading:\n");
	i = read(fd,&tmp,MAX_NUM-1);
	    printf("read only %d of set pins  - readen %d\n",MAX_NUM+10,i);
	i = read(fd,&tmp,MAX_NUM+10);
	    printf("read only %d of set pins  - readen %d\n",MAX_NUM+10,i);
	for (i=0;i < MAX_NUM; i++)
		if (tmp[i] > 0) printf("1 "); else printf("0 ");
	printf("\n");
	for (i=0;i < MAX_NUM; i++)
		printf("%c ",tmp[i]+'0');
	printf("\n");
	
	return 0;
	}
