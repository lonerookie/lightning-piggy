// Code for the Lightning Piggy running on the TTGO LilyGo 2.66 inch ePaper (296X152) hardware.
// https://www.lightningpiggy.com/
//
// Tested with:
// ============
// Arduino IDE version 1.8.13
// ESP32 Board Support version 2.0.6
// Preferences -> Compiler warnings: Default
// Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module
// Tools -> Upload Speed: 921600
// Tools -> CPU Frequency: 240Mhz
// Tools -> Flash Frequency: 80Mhz
// Tools -> Flash Mode: QIO
// Tools -> Flash Size: 4MB (32Mb)
// Tools -> Partition Scheme: Default 4MB with spiffs (1.2MB APP, 1.5MB SPIFFS)
// Tools -> Core Debug Level: Warn
// Tools -> PSRAM: Disabled
// Tools -> Port: /dev/ttyACM0
//
// Make sure the Arduino IDE has permissions to access the serial port.
// Hint: sudo chmod -f 777 /dev/ttyACM* /dev/ttyUSB*
//
// On the lnbits webpage:
// - activate the LNURLp extension
// - click on the LNURLp extension
// - click "NEW PAY LINK"
// - untick "fixed amount"
// - set minimum amount: 1
// - set maximum amount: 100000000
// - set currency to "satoshis"
// - click "Advanced options"
// - set "Comment maximum characters" to 128
// - set "Webhook URL" to https://www.lightningpiggy.com/paymentreceived
// - set a "Success message", like: Thanks for sending sats to my piggy
//
// Wakeup reasons:
// - rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT): after flashing firmware or pushing the reset button
// - rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT): after wakeup from hibernate
// - rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT): using the sliding switch
// - watchdog reset

// TODO:
// - metrics

#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// for rtc_gpio_pullup_dis and rtc_gpio_pulldown_en
#include "driver/rtc_io.h"
// for rtc_get_reset_reason
#include <rom/rtc.h>

#define LILYGO_T5_V266
#include <boards.h>

#include "qrcoded.h"
#include "logos.h"
#include "config.h"

// Display stuff:
#include <GxEPD.h>
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

// Fonts after display:
#include "Fonts/LatoMedium8pt.h"
#include "Fonts/LatoMedium12pt.h"
#include "Fonts/LatoMedium18pt.h"
#include "Fonts/LatoMedium20pt.h"
#include "Fonts/LatoMedium26pt.h"

#define BUTTON_PIN_BITMASK 4294967296 // 2^32 means GPIO32

// Global variables for display
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

void setup()
{
    Serial.begin(115200);
    Serial.println("Lightning Piggy version 1.2.0 starting up");

    // turn on the green LED-IO12 on the PCB, to show the board is on
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    print_reset_reasons();
    print_wakeup_reason();

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();

    // partial update to full screen to preset for partial update of box window (this avoids strange background effects)
    // this needs to be done before setRotation, otherwise still faint/missing pixels, even with using_rotation = true
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    display.fillScreen(GxEPD_WHITE); // erase entire display, otherwise old stuff might still be (faintly) there
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1); // display is used in landscape mode

    // display a border to simulate the new low resolution display
    displayBorder();

    // update display (with delay if battery low warning)
    if (checkShowLowBattery()) {
      display.update();
      delay(5000);
    } else {
      display.update();
    }

    // piggy logo to show the board is started
    display.drawBitmap(piggyLogo, 0, 0, 104, 104, GxEPD_WHITE);
    display.updateWindow(0, 0, 104, 104, true);

    Serial.println("Connecting to " + String(ssid));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected, IP address: " + WiFi.localIP());

    // bitcoin logo to show wifi is connected
    display.drawBitmap(epd_bitmap_Bitcoin, displayWidth() - 104, 0, 104, 104, GxEPD_WHITE);
    display.updateWindow(displayWidth() - 104, 0, 104, 104, true);
}


void loop() {
    display.fillScreen(GxEPD_WHITE);
    int balance = getWalletBalance();
    displayVoltageAndLowBatteryWarning();
    displayBorder();
    printBalance(balance);
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false); // seems needed to avoid artifacts later on when doing partial draws
    display.update();

    String lnurlp = getLNURLp();
    if (lnurlp == "null") {
      Serial.println("Warning, could not find lnurlp link for this wallet, did you create one?");
      Serial.println("You can do so by activating the LNURLp extension in LNBits, clicking on the extension, and clicking 'NEW PAY LINK'");
      Serial.println("You probably don't want to go for 'fixed amount', but rather for any amount.");
    } else {
        showLNURLpQR(lnurlp);
    }
    getLNURLPayments(3);
    display.update();

    hibernate(6 * 60 * 60);
}
