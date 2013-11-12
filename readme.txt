Main goal to create this module was to improve speed and make gpio easier for me. 
Advantages of this kernel module are:
- faster gpio reading and writing (by just one call to read or write device) for three pins it gives about 4x advantage
- gpio pins are just numbered like You have it in Your board config, and not like for sunxi gpio with additional label like PH0-1 etc
- only one open file descriptor not one or two per pin

TODO list:
- cleanup code and make it much safer 
- try to make soft slow IRQ on gpios i.e on poll call and time to time check
- gpio requesting code on IOCTL - for compatibility with never kernels
- new ioctls for i.e one or more pin change

