# :joystick: Universal RC Joystick

Contents:

* [What is it?](#what-is-it)
* [What do I need?](#what-do-i-need)
* [How to flash?](#how-to-flash)
* [How to use?](#how-to-use)
* [Technical details](#technical-details)

## What is it?

Universal RC Joystick (URCJ) is a _firmware_ for the popular STM32F103 board (aka blue pill) which allows you to use a RC receiver with any device/software that supports USB HID Joystick.

![image](https://user-images.githubusercontent.com/9365881/159189774-b3722800-77d2-4805-9dbb-2d55494caff3.png)

Your transmitter sends wirelessly the signal to the receiver, the receiver passes it to the STM32 board, which presents the data to the host device as a standard USB HID Joystick.

## What do I need?

1. A STM32F103 Blue pill board. Clones can be found for very cheap ($2-$3 at [AliExpress](https://www.aliexpress.com/wholesale?SearchText=stm32f103+board) for example). The STM32F103**C6**T6 (note the **C6**) is cheaper and also works.
2. A micro USB cable
3. ST-Link or UART converter for flashing (see [below](#how-to-flash)).
4. A RC receiver

The following protocols are currently supported:

Digital:
* SBUS (inverted and uninverted) – FrSky, Futaba and others
* IBUS – FlySky (iA6 is also supported. See [here](http://endoflifecycle.blogspot.com/2016/10/flysky-ia6-ibus-setup.html) for how to get the digital signal)
* DSM – Spektrum
* FPort (inverted and uninverted) – FrSky

Analog:
* PPM – vendor independent. Note that digital protocol is always preferable because of its much lower latency

## How to flash?

First head over to the [releases](../../releases) section and download the appropriate `.hex` file depending on whether you have the **C6** or **C8** board.

### 1. Using ST-Link

Using ST-Link is the standard way for flashing STM32 chips. There are many tutorials that explain how to do it (like [this](https://www.youtube.com/watch?v=KgR3uM21y7o)).

### 2. Using USB TTL UART converter

Most people don't really need to flash STM32 chips in their day to day life so buying a ST-Link to use it only once is really an overkill. STM32 chips can also be programmed through UART serial interface. It's much more likely that you already have such a converter, such as FTDI adapter. There are [many](https://www.aliexpress.com/wholesale?SearchText=usb+uart+ttl) options.

![image](https://user-images.githubusercontent.com/9365881/159186940-0fa6ec3a-80aa-432e-b659-e430d1e9d8ff.png)

#### a. Standard UART converter
1. You'll need to move the `BOOT0` jumper to the `1` position
2. Connect the `A9` pin (the STM32 _sends_ data out through that pin) to the `RX` pin of the converter
3. Connect the `A10` pin (the STM32 _receives_ data through that pin) to the `TX` pin of the converter
4. Connect `GND`
5. Connect `5V`

#### b. Using Arduino for its UART converter
If you don't have a UART converted but you have an Arduino board, you can use that too because it comes with UART converter. But there are few things to take into account:
> !!! Please read and understand the points below, otherwise you may damage your devices!
1. Flash an _empty sketch_ to the Arduino first! This is needed to make sure that all pins are in their default _input_ state (high impedance). If some of the pins are set to outputs you may burn something (if two pins which are connected together, output `0` and `1` respectively, they are practically shorted!). We need the Arduino board _only_ for its UART converter.
2. You need to connect in the following way:

STM32 | Arduino
---|---
A9 (TX) | TX
A10 (RX) | RX

It may seem strange that we connect `TX` to `TX` and `RX` to `RX`. The reason is because the labels on the Arduino board are from the standpoint of the Atmega chip. The `TX` pin is internally connected to the `RX` pin of the UART chip (and that's where we want to hook the STM32's `TX`).

#### Software for flashing
Assuming that your UART converter is connected, there are few ways in which you can do the actual flashing.
1. Use a tool like [stm32loader](https://github.com/jsnyder/stm32loader). You can see a guide [here](https://paramaggarwal.medium.com/programming-an-stm32f103-board-using-usb-port-blue-pill-953cec0dbc86)
2. But there's easier way if you already have the [Betaflight configurator](https://github.com/betaflight/betaflight-configurator) installed.

![image](https://user-images.githubusercontent.com/9365881/159187938-b4d3db40-c29b-4623-ae4a-e9c209f97431.png)

1. Go to the `Firmware Flasher` tab
2. Select the COM port of your UART converter
3. Load the `.hex` file
4. `No reboot sequence` should be checked
5. It's recommended that `Full chip erase` is checked
6. Check `Manual baud rate`
7. This is not strictly necessary but I found out that flashing works more reliably at `19200` bps
8. Flash. If it doesn't work try resetting the STM32 board with its `RESET` button and try again

> Note! There are some buggy clones which have faulty `BOOT0` pin. To check if that is the case, make sure your board is _powered_ and `BOOT0` is in the `1` position then turn the board around and measure the voltage across the `R3` resistor. Normally it should read 0V. If you see something like 2.3V you have one from the faulty batch.
>
> Don't worry you can still flash but you'll need some manual work. Take a pair of tweezers or something else with which you can conveniently short the resistor (make sure your tweezers are conductive - verify that there's continuity between the tips of the tweezers with your multimeter).
> Hold the board with one hand and prepare a finger on the `RESET` button. With your other hand short the resistor and _then_ press the `RESET` button. After that you can remove the shorting. The chip should now be in bootloader mode and you can perform the flashing procedure.
> 
> ![image](https://user-images.githubusercontent.com/9365881/159188923-cbd67270-7a5f-4a95-95a9-5b6df822e348.png)

After you're done return `BOOT0` to its original `0` position.

## How to use?

### 1. Connect the receiver

After the firmware has been flashed you need to connect your receiver:

Receiver | STM32
---|---
+5V | 5V
GND | GND
DATA | B5

> Note that if you're using DSM receiver you should use 3.3V instead of 5V. Also the receiver should be already bound to the transmitter.

### 2. Bridge `A3` and `A4`

You can solder or use a jumper:

![image](https://user-images.githubusercontent.com/9365881/159189249-75d95506-99ff-4f68-a83d-46f64c23e016.png)

### 3. Plug the micro USB cable

The firmware uses the standard ST provided hardware ids - VID 0483 PID 5710. It should immediately be recognized as standard USB Input Device. It should also be visible in the Game Controllers settings:

![image](https://user-images.githubusercontent.com/9365881/159189432-3feb84a5-44be-423c-a426-3ccae92198ed.png)

> Note: since we use the standard hardware ids it's possible that other devices using the same ids have been connected to your PC at some prior point. If this is the case you may see different name for the Joystick (for example `FrSky Taranis Joystick`, which uses the same standard ids). Don't worry about this.

> Note: some of the latest Windows 10 updates seem to automatically install another driver for these hardware ids (which unfortunately doesn't work):
>
> ![image](https://user-images.githubusercontent.com/9365881/159189601-c118d713-7471-4c71-8e0d-e3e6feea6cfc.png)
>
> If your situation is like this, please follow [this](https://www.youtube.com/watch?v=SDnRSL3rDwM) guide.

### 4. Set up your transmitter

URCJ is a zero-configuration tool. The type of the receiver is _automatically_ detected. There are two modes of the LED:

LED | Meaning
---|---
Blinking | No connection with the receiver
Continuous | The receiver is recognized and connected

It is recommended that you create a separate model in your transmitter for use with URCJ. Consult your transmitter's documentation on how to do this. Make sure all controls that you need are assigned to RC channels.

The RC channels are mapped to joystick axes/buttons in the following way:

RC Channel | USB Joystick Axis/Button
---|---
1 | X
2 | Y
3 | Z
4 | Rx
5 | Ry
6 | Rz
7 | Button 1
8 | Button 2
9 | Button 3
10 | Button 4

Open [https://gamepad-tester.com/](https://gamepad-tester.com/) with a recent browser. You should be seeing your joystick there.  When you move your sticks you should be seeing the raw values of the axes ranging between `-1` to `1` with `0` at the center. Adjust your endpoints and center (sometimes called subtrim) if needed.

Buttons are considered to be pressed when the channel's value is above halfway (> 1500)

### 5. Enjoy!

At this point everything should be working and you are ready to use URCJ with your favorite sims. Note that even though the examples above are for Windows, URCJ should work on _any_ platform (Mac, Linux, Android, etc.) as long as it supports USB HID devices.

## Technical details

The skeleton of the firmware is generated with [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) (project file is `urcjoy.ioc`). Development is done in [Visual Studio Code](https://code.visualstudio.com/) together with the [PlatformIO extension](https://platformio.org/platformio-ide).

Turning the STM32 board into a USB HID device is standard code. Reading the receiver is also straightforward. The only difficulty is that the SBUS protocol uses inversed polarity of the UART signal. Unfortunately the `F103` series _does not_ support _software_ inversion of the signal. The employed solution is the following:

![image](https://user-images.githubusercontent.com/9365881/159191265-81d95bb6-473d-441d-9bbf-380675655e74.png)

There are two different modes of operation:
1. PPM. In this mode `DATA_IN` is configured as Timer3 Channel2 and used to capture the length of the PPM pulses.
2. Digital protocols. Here `DATA_IN` is a regular GPIO Input. Any changes to that pin are caught by the interrupt handler. Then the state is immediately written to `DATA_OUT` but we have two cases:

   a. Inverted signal - we write to `DATA_OUT` the _inverted_ value (that is, if `DATA_IN` is `0` we write `1` to `DATA_OUT` and vice versa

   b. Normal signal - we write the _same_ value to `DATA_OUT`

Then the externally bridged signal is read by `USART2`.

Once the RC data is decoded it is sent over USB.

## Like it?
If this software brought a smile on your face, you may shine back if you feel like it: [![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=L5789HZB5NAX4&lc=BG&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted)\
Thank you!!!
