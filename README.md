## Overview
![Overview](https://raw.githubusercontent.com/EiichiroIto/m5stickcUartUdpBridge/master/images/microbit-tello.png)

## Connection
    micro:bit      M5StickC
       GND     ----   GND
       3V      ----   3V3
       Pin0    ----   G36
    radio-UART     UART-UDP

## micro:witch program (radio-UART bridge program)
![microwitch1](https://raw.githubusercontent.com/EiichiroIto/m5stickcUartUdpBridge/master/images/tello-uart.gif)

## MicroPython program (radio-UART bridge program)
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


