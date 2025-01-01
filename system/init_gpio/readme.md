The two GPIO RasPi pins are used to control the Reset and Boot0 pins of the STM.
When the system starts, they should be set to the low state.

In the init_gpio.h needs to specify which pins will be used.
For example for socket's pins 16 and 18 ( GPIO 23 and GPIO24 ) needs to set:

```
#define GPIO_A 23
#define GPIO_B 24
```

After compilation, need to add it to one of the startup methods. 
For example to the `/etc/rc.local`:
```
/usr/bin/init_gpio &
```

