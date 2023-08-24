// Code for the Lightning Piggy running on the TTGO LilyGo 2.13 and 2.66 inch ePaper hardware.
// See README.md for tips and tricks.

// For 2.13 inch LilyGo ePaper:
#define LILYGO_T5_V213

// For 2.66 inch LilyGo ePaper (end of life), uncomment the LILYGO_T5_V266 line below and comment out the LILYGO_T5_V213 line above.
//#define LILYGO_T5_V266

#include <boards.h>

#include "logos.h"
#include "config.h"

// Display stuff:
#include <GxEPD.h>
#ifdef LILYGO_T5_V266
  #include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" black/white ePaper display by DKE GROUP
#elif defined LILYGO_T5_V213
  #include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" black/white ePaper display by DKE GROUP
//#include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" black/white ePaper display by GoodDisplay with 2 additional gray levels
#endif
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
    Serial.println("Staring Lightning Piggy " + getFullVersion());

    // turn on the green LED-IO12 on the PCB, to show the board is on
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    print_reset_reasons();
    print_wakeup_reason();

    setup_display();

    // allow some time to show low battery warning
    if (displayVoltageWarning()) delay(5000);

    // piggy logo indicates board is starting
    showLogo(piggyLogo, 104, 104, ((displayWidth() / 2) - 104) / 2, 0);

    displayFit("Connecting to " + String(ssid) + "...", 0, 103, displayWidth(), displayHeight(), 2); // somehow 104 causes yPos 120 and that causes the last line to be cut off
    #ifdef LILYGO_T5_V266
    display.update();
    #endif

    #ifndef DEBUG
    connectWifi();
    #endif

    // bitcoin logo indicates wifi is connected
    showLogo(epd_bitmap_Bitcoin, 104, 104, (((displayWidth() / 2) - 104) / 2) + (displayWidth() / 2), 0);
}


void loop() {
    int balance = getWalletBalance();

    #ifdef LILYGO_T5_V266
    display.fillScreen(GxEPD_WHITE); // erase entire display, otherwise old stuff might still be (faintly) there
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false); // seems needed to avoid artifacts later on when doing partial draws
    #else
    // erase the previous screen (bootup with logos)
    display.fillRect(0, 0, displayWidth(), displayHeight(), GxEPD_WHITE);
    // TODO: try this: display.fillScreen(GxEPD_WHITE); // erase entire display, otherwise old stuff might still be (faintly) there
    display.updateWindow(0, 0, displayWidth(), displayHeight(), true);
    #endif

    // build the new screen:
    int yAfterBalance = printBalance(balance);

    displayHealthAndStatus();

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

    displayVoltageWarning();

    if (wifiConnected()) checkShowUpdateAvailable();

    #ifdef LILYGO_T5_V266
    display.update();
    #endif

    hibernate(6 * 60 * 60);
}
