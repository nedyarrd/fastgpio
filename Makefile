obj-m += fastgpio.o

all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 

test:
	gcc ioctl-test.c -o ioctl-test
	./ioctl-test
	dmesg | tail -n 5
	
