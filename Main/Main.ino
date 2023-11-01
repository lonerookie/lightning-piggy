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

#include "Constants.h"

// Global variables for display
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Lightning Piggy " + getFullVersion());

    // turn on the green LED-IO12 on the PCB, to show the board is on
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    print_reset_reasons();
    print_wakeup_reason();

    setup_display();

    // allow some time to show low battery warning
    if (displayVoltageWarning()) {
       delay(5000);
    }

    displayFit("Behold, today's pearl of wisdom", 0, 20, displayWidth(), 40, 1); 
    displayFit("from Dad is...:", 0, 40, displayWidth(), 60, 1); 

    String slogan = getRandomBootSlogan();
    Serial.println("slogan " + slogan);
    displayFit(slogan, 0, 65, displayWidth(), 160, 4); 

    delay(3000);

    // erase the screen 
    display.fillScreen(GxEPD_WHITE);
    updateWindow(0, 0, displayWidth(), displayHeight());

    String baseConnectMsg = "Connecting to " + String(ssid) + "...";
    String connectingMsg = baseConnectMsg + "    ";
    displayFit(connectingMsg, 0, 1, displayWidth(), 20, 1); // somehow 104 causes yPos 120 and that causes the last line to be cut off

    // piggy logo indicates board is starting
    showLogo(piggyLogo, 104, 104, ((displayWidth() / 2) - 104) / 2, 16);

    #ifndef DEBUG
    connectWifi();
    #endif

    displayHealthAndStatus();

    String connectedMsg = baseConnectMsg + " OK!";
    displayFit(connectedMsg, 0, 1, displayWidth(), 20, 1); // somehow 104 causes yPos 120 and that causes the last line to be cut off

    delay(1000);
}

void loop() {    
    // erase the setup screen 
    display.fillScreen(GxEPD_WHITE);
    updateWindow(0, 0, displayWidth(), displayHeight());

    showLogo(epd_bitmap_Bitcoin, 44, 44, (displayWidth() / 2) + 76, 61);

    int balance = getWalletBalance();

    // build the new screen:
    int yAfterBalance  = 0;
    bool balanceOk = balance != NOT_SPECIFIED;

    if (balanceOk) {
       yAfterBalance = printBalance(balance);
    }
    else {
       displayBoldMessage("GET WALLET ERROR", 30);
    }

    String lnurlp = getLNURLp();
    int xBeforeLNURLp = displayWidth();
    if (lnurlp == "null") {
      Serial.println("Warning, could not find lnurlp link for this wallet, did you create one?");
      Serial.println("You can do so by activating the LNURLp extension in LNBits, clicking on the extension, and clicking 'NEW PAY LINK'");
      Serial.println("You probably don't want to go for 'fixed amount', but rather for any amount.");
    } else {
        xBeforeLNURLp = showLNURLpQR(lnurlp);
    }

    getLNURLPayments(2, xBeforeLNURLp - 10, yAfterBalance);

    float btcPrice = getBitcoinPrice();

    String currentTime = getTimeFromNTP(); 

    bool btcPriceOk = btcPrice != NOT_SPECIFIED;
    if (!btcPriceOk) {
      displayBoldMessage("GET BTC PRICE ERROR", 30);
    }
    if (btcPriceOk && balanceOk) {
        float balanceValue = btcPrice / 100000000 * balance;
        Serial.println("balanceValue" + String(balanceValue, 2));
        displayBoldMessage(floatToString(balanceValue, 2) + getCurrentCurrencyCode() + " (" + formatFloatWithSeparator(btcPrice) + getCurrentCurrencyCode() + ")", displayHeight() - 4);
    }

    displayTime(currentTime);
    displayVoltageWarning();

    if (wifiConnected()) checkShowUpdateAvailable();

    int sleepTimeSeconds = sleepTimeMinutes * 60;
    hibernate(sleepTimeSeconds);
}
