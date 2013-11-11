if [ ! -d /sys/class/gpio/gpio3_pd0/ ]; then
        echo 3 > /sys/class/gpio/export
        fi
if [ ! -d /sys/class/gpio/gpio4_pd1/ ]; then
        echo 4 > /sys/class/gpio/export
        fi
if [ ! -d /sys/class/gpio/gpio5_pd2/ ]; then
        echo 5 > /sys/class/gpio/export
        fi

