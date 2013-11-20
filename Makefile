KCFLAGS="-O3 -pipe -fomit-frame-pointer -mtune=native -mcpu=native -mfpu=neon-vfpv4 -g -DDEBUG" 

obj-m += fastgpio.o


all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

clean:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 
	rm ioctl-test?

test:
	gcc -pg -g -D FAST_LANE ioctl-test.c -o ioctl-test1
	gcc -pg -g -D NORMAL_LANE ioctl-test.c -o ioctl-test2
	opcontrol --deinit
	opcontrol --separate=kernel
	opcontrol --setup --vmlinux=/lib/modules/`uname -r`/build/vmlinux 
	opcontrol --init
	opcontrol --reset
	opcontrol --start
	./ioctl-test1 
	./profile.sh
	./ioctl-test2
	opcontrol --stop
	opreport -l ./ioctl-test1
	opreport -l ./ioctl-test2
	dmesg | tail -n 5
	


test2:
	./profile.sh
	gcc -pg -g -D FAST_LANE ioctl-test.c -o ioctl-test1
	gcc -pg -g -D NORMAL_LANE ioctl-test.c -o ioctl-test2
	./ioctl-test1 
	./ioctl-test2
	dmesg | tail -n 5
	
test3:
	gcc -pg -g -D FAST_LANE ioctl-test.c -o ioctl-test1
	rmmod fastgpio
	modprobe clean-gpio 
	rmmod clean-gpio
	insmod fastgpio.ko
	./ioctl-test1