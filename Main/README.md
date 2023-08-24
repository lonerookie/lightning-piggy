# Lightninig Piggy

Code for the Lightning Piggy running on the TTGO LilyGo 2.13 and 2.66 inch ePaper hardware.

**Tested with:**

- Arduino IDE version 1.8.13
- ESP32 Board Support version 2.0.6
- Preferences -> Compiler warnings: Default
- Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module
- Tools -> Upload Speed: 921600
- Tools -> CPU Frequency: 240Mhz
- Tools -> Flash Frequency: 80Mhz
- Tools -> Flash Mode: QIO
- Tools -> Flash Size: 4MB (32Mb)
- Tools -> Partition Scheme: Default 4MB with spiffs (1.2MB APP, 1.5MB SPIFFS)
- Tools -> Core Debug Level: Warn
- Tools -> PSRAM: Disabled
- Tools -> Port: /dev/ttyACM0

Make sure the Arduino IDE has permissions to access the serial port:

`sudo chmod -f 777 /dev/ttyACM* /dev/ttyUSB*`

**On the lnbits webpage:**

- activate the LNURLp extension
- click on the LNURLp extension
- click "NEW PAY LINK"
- untick "fixed amount"
- set minimum amount: 1
- set maximum amount: 100000000
- set currency to "satoshis"
- click "Advanced options"
- set "Comment maximum characters" to 128
- set "Webhook URL" to https://p.lightningpiggy.com/ (optional, for anonymous usage metrics)
- set a "Success message", like: Thanks for sending sats to my piggy



