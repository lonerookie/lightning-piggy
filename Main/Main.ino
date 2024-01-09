// Code for the Lightning Piggy running on the TTGO LilyGo 2.13 and 2.66 inch ePaper hardware.
// See README.md for tips and tricks.

// For 2.13 inch LilyGo ePaper:
#define LILYGO_T5_V213

// For 2.66 inch LilyGo ePaper (end of life), uncomment the LILYGO_T5_V266 line below and comment out the LILYGO_T5_V213 line above.
//#define LILYGO_T5_V266

#include <boards.h>
#include <string.h>

#include "logos.h"
#include "config.h"
#include "pin_config.h"

#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */

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

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1

TFT_eSPI tft = TFT_eSPI();
#define WAIT 1000
unsigned long targetTime = 0; // Used for testing draw times

#if defined(LCD_MODULE_CMD_1)
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif


void setup() {
    Serial.begin(115200);
    Serial.println("Starting Lightning Piggy " + getFullVersion());

    tft.begin();
    #if defined(LCD_MODULE_CMD_1)
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7f); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
#endif


    tft.setRotation(3);
    tft.setSwapBytes(true);
    //tft.pushImage(0, 0, 320, 170, (uint16_t *)img_logo);
    delay(2000);
    
    // turn on the green LED-IO12 on the PCB, to show the board is on
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    print_reset_reasons();
    print_wakeup_reason();

    setup_display();

    #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
#else
    ledcAttach(PIN_LCD_BL, 200, 8);
    ledcWrite(PIN_LCD_BL, 255);
#endif

    // allow some time to show low battery warning
    if (displayVoltageWarning()) {
       delay(5000);
    }

    if (settingLanguage == LANGUAGE_EN_US) {
      displayFit("Behold, today's pearl of wisdom", 0, 20, displayWidth(), 40, 1); 
      displayFit("from Dad is...:", 0, 40, displayWidth(), 60, 1); 
    }
    else if (settingLanguage == LANGUAGE_DA){
      displayFit("Klar! Her kommer dagens perle af", 0, 20, displayWidth(), 40, 1); 
      displayFit("visdom fra far...:", 0, 40, displayWidth(), 60, 1); 
    }

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
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
    tft.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    tft.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    tft.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);

    tft.fillScreen(TFT_BLACK);
    tft.drawString("890:.", 0, 0, 7);
    tft.drawString("", 0, 60, 7);

    // erase the setup screen 
    //display.fillScreen(GxEPD_WHITE);
    //updateWindow(0, 0, displayWidth(), displayHeight());

    //showLogo(epd_bitmap_Bitcoin, 40, 40, (displayWidth() / 2) + 78, 67);

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
    if (btcPriceOk && balanceOk) {
        float balanceValue = btcPrice / 100000000 * balance;
        String balanceValueToShow = floatToString(balanceValue, 2) + getCurrentCurrencyCode();
        Serial.println("balanceValue" + balanceValueToShow);

        String currentBtcPriceToShow = formatFloatWithSeparator(btcPrice);
        bool addCurrencyCodeToCurrentBtcPrice = strlen(currentBtcPriceToShow.c_str()) <= 6; 
        if (addCurrencyCodeToCurrentBtcPrice) {
           currentBtcPriceToShow += getCurrentCurrencyCode();
        }
        displayBoldMessage(" " + balanceValueToShow + " (" + currentBtcPriceToShow + ")", displayHeight() - 4);
    }

    displayTime(currentTime);
    displayVoltageWarning();

    if (wifiConnected()) checkShowUpdateAvailable();

    int sleepTimeSeconds = sleepTimeMinutes * 60;
    //hibernate(sleepTimeSeconds);
}
