#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include "fastgpio_ioctl.h"


#define MAX_NUM 3
#define ITERATIONS 5000000
int main(int argc,char *argv[])
	{
	char *file_name = "/dev/fastgpio";
	char tmp[10];
	char tmp2[10];
	int fd,i,fd1,fd2,fd3;
	time_t start,end;
	double dif;

	for (i=0;i<10;i++)
		tmp[i] = 0;
	gpio_ioctl gpio_sw;
	gpio_sw.number = MAX_NUM;
	memset(&gpio_sw.pins,MAX_GPIO,sizeof(gpio_sw.pins));
	gpio_sw.pins[0] = 4;
	gpio_sw.pins[1] = 3;
	gpio_sw.pins[2] = 5;
	gpio_sw.dir[0] = 0;
	gpio_sw.dir[1] = 0;
	gpio_sw.dir[2] = 0;

	printf("dev/fastgpio open\n");
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
	printf("dev/fastgpio ioctl READ_PINS\n");
	if (ioctl(fd,FASTGPIO_READ_PINS,&gpio_sw))
		{
		perror("ioctl READ_PINS failed\n");
		return 3;
		}
	if (ioctl(fd,FASTGPIO_SET_DIR,&gpio_sw))
		{
		perror("ioctl SET_DIR failed\n");
		return 3;
		}

	printf("testing file reading:\n");
	i = read(fd,tmp,MAX_NUM-1);
	    printf("read only %d of set pins  - readen %d\n",MAX_NUM+10,i);
	i = read(fd,tmp,MAX_NUM+10);
	    printf("read only %d of set pins  - readen %d\n",MAX_NUM+10,i);
	for (i=0;i < MAX_NUM; i++)
		if (tmp[i] == 1) printf("1 "); else printf("0 ");
	printf("\n");
#ifdef FAST_LANE
	tmp[0] = 1;
	tmp[1] = 1;
	tmp[2] = 1;
	tmp2[0] = 0;
	tmp2[1] = 0;
	tmp2[2] = 0;
	i = write(fd,tmp,3);
	printf("writen %d bytes to device\n",i);
	if (argc ==  1)
	{
	time(&start);
	for (i=0;i<ITERATIONS;i++)
		{
		write(fd,tmp,3);
		write(fd,tmp2,3);
		}
	time(&end);
	dif = difftime (end,start); 
	printf ("Elasped time is %.2lf seconds.\n", dif );
	}
#endif
#ifdef NORMAL_LANE
	fd1 = open("/sys/class/gpio/gpio3_pd0/value",O_RDWR);
	fd2 = open("/sys/class/gpio/gpio4_pd1/value",O_RDWR);
	fd3 = open("/sys/class/gpio/gpio5_pd2/value",O_RDWR);
	tmp[0] = '1';
	tmp[1] = '1';
	tmp[2] = '1';

	tmp2[0] = '0';
	tmp2[1] = '0';
	tmp2[2] = '0';
	time(&start);
	for (i=0;i<ITERATIONS;i++)
		{
		write(fd1,tmp,1);
		write(fd2,tmp,1);
		write(fd3,tmp,1);
		fsync(fd1);
		fsync(fd2);
		fsync(fd3);
		write(fd1,tmp2,1);
		write(fd2,tmp2,1);
		write(fd3,tmp2,1);
		fsync(fd1);
		fsync(fd2);
		fsync(fd3);
		}
	time(&end);
	dif = difftime (end,start); 
	printf ("Elasped time is %.2lf seconds.", dif );
	
#endif 	
	return 0;
	}
