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
// - set "Webhook URL" to https://p.lightningpiggy.com/
// - set a "Success message", like: Thanks for sending sats to my piggy
//
// TODO:
// - metrics

#include <WiFiClientSecure.h>

#define LILYGO_T5_V266
//#define LILYGO_T5_V213
#include <boards.h>

#include "logos.h"
#include "config.h"

// Display stuff:
#include <GxEPD.h>
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" black/white ePaper display by DKE GROUP
//#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" black/white ePaper display by DKE GROUP
//#include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" b/w  form GoodDisplay 4-color
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

// Fonts after display:
#include "Fonts/LatoMedium8pt.h"
#include "Fonts/LatoMedium12pt.h"
#include "Fonts/LatoMedium18pt.h"
#include "Fonts/LatoMedium20pt.h"
#include "Fonts/LatoMedium26pt.h"

// Global variables for display
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

void setup() {
    Serial.begin(115200);
    Serial.println("Lightning Piggy version 1.3.0 starting up");

    // turn on the green LED-IO12 on the PCB, to show the board is on
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    print_reset_reasons();
    print_wakeup_reason();

    setup_display();
    whiteDisplay();

    // update display (with delay if battery low warning)
    if (checkShowLowBattery()) {
      display.update();
      delay(5000);
    } else {
      display.update();
    }

    // piggy logo indicates board is starting
    int logoheight = 104;
    int logowidth = 104;
    int logoheightcentered = (displayHeight() - logoheight) / 2;
    int logowidthcentered = ((displayWidth() / 2) - logowidth) / 2;
    display.drawBitmap(piggyLogo, logowidthcentered, logoheightcentered, logowidth, logoheight, GxEPD_WHITE);
    display.updateWindow(logowidthcentered, logoheightcentered, logowidth, logoheight, true);

    #ifndef DEBUG
    Serial.println("Connecting to " + String(ssid));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
    #endif

    // bitcoin logo indicates wifi is connected
    logowidthcentered = (((displayWidth() / 2) - logowidth) / 2) + (displayWidth() / 2);
    display.drawBitmap(epd_bitmap_Bitcoin, logowidthcentered, logoheightcentered, logowidth, logoheight, GxEPD_WHITE);
    display.updateWindow(logowidthcentered, logoheightcentered, logowidth, logoheight, true);
}


void loop() {
    whiteDisplay();

    displayVoltageAndLowBatteryWarning();
    int yAfterBalance = printBalance(getWalletBalance()) + 6;
    #ifdef LILYGO_T5_V266
    yAfterBalance += 10;
    #endif
    display.update();

    String lnurlp = getLNURLp();
    int xBeforeLNURLp = displayWidth();
    if (lnurlp == "null") {
      Serial.println("Warning, could not find lnurlp link for this wallet, did you create one?");
      Serial.println("You can do so by activating the LNURLp extension in LNBits, clicking on the extension, and clicking 'NEW PAY LINK'");
      Serial.println("You probably don't want to go for 'fixed amount', but rather for any amount.");
    } else {
        xBeforeLNURLp = showLNURLpQR(lnurlp);
    }
    getLNURLPayments(2, xBeforeLNURLp, yAfterBalance);
    display.update();

    hibernate(6 * 60 * 60);
}
