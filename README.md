# canfilter

This is a radar can message interceptor designed for Toyota NODSU Camry/CH-R. It is based on an STM32 mileage manipulator board.

- or CH-R, it enables full speed ACC with openpilot.
- For Camry Hybrid/XLE/XSE, it enables full speed ACC with stock system and openpilot.
- For Camry LE/SE, it bypass 28mph(45kph) speed limit. But the ABS module ignores braking request under 15mph(24kph). This limit applies to both stock system and openpilot.

![Radar Interceptor](https://github.com/Smartype/canfilter/blob/master/board/resources/radar-interceptor.jpg?raw=true)

## Features
- boot loader from commaai panda project, to support firmware updating on CAN bus
- crash detection, on crash it will switch to passthrough failsafe mode
- mode selector, it can be switched into a passthrough mode, then it is invisable on CAN bus
- init magic, enable ACC_CONTROL ACC_TYPE override, which make PCM believe the ACC is full speed range
- manually speed lockout, lock the engage speed at 30mph, disengage speed at 25mph
- fake low speed lead in stock ACC mode, so that you can engage at when there is no lead car
- control with ACC_CONTROL, when ACC_CONTROL is seen on can BUS 0, it mutes the stock radar ACC_CONTROL msg.
- control with ACC_CONTROL_SAFE, when ACC_CONTROL_SAFE is seen on can BUS 0, it mutes stock radar ACC_CONTROL msg, and if AEB engages, it mutes ACC_CONTROL_SAFE and allow stock ACC_CONTROL msg
- ACC_CONTROL_COPY, it mirrors the stock ACC_CONTROL msg with a different msg id, so it can be logged/used in openpilot(for example, the DISTANCE_REQ bit)
- stock ACC_CONTROL alert override, if OP long is engaged, while the stock ACC emits an alert(CUT IN), it will be merged into final ACC_CONTROL msg

## Wiring
Please check pictures in board/resources. The radar has 2 can buses, CAN1 is what we are going to intercept.

The CAN1 on filter board should be connected to CAN gateway(the car), CAN2 to be connected to radar CAN1.

## License
Code in this repository is released under the MIT license.

USING ANYTHING FROM THIS REPOSITY IS AT YOUR OWN RISK!


