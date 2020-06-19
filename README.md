## Connection
    M5StickC   micro:bit
       GND ---    GND 
       3V3 ---    3V
       G36 ---    Pin0

## micro:witch program
![Entire Screen1](https://raw.githubusercontent.com/EiichiroIto/m5stickcUartUdpBridge/master/images/tello-uart.gif)

## MicroPython program
```
import microbit
from microbit import uart
import radio

a = 0
radio.on()
radio.config(group=0)
microbit.sleep(3000)
microbit.display.scroll(str('start'))
uart.init(baudrate=115200,tx=microbit.pin0,rx=microbit.pin1)
while True:
    a = radio.receive()
    if not(a is None):
        uart.write(a)
```


