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

// for rtc_gpio_pullup_dis and rtc_gpio_pulldown_en
#include "driver/rtc_io.h"
// for rtc_get_reset_reason
#include <rom/rtc.h>


#define LILYGO_T5_V266
#include <GxEPD.h>
#include <boards.h>
#include "qrcoded.h"
#include "logos.h"
#include "config.h"

#include <WiFiClientSecure.h>

#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP

#include "Fonts/LatoMedium8pt.h"
#include "Fonts/LatoMedium12pt.h"
#include "Fonts/LatoMedium18pt.h"
#include "Fonts/LatoMedium20pt.h"
#include "Fonts/LatoMedium26pt.h"

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

#define BUTTON_PIN_BITMASK 4294967296 // 2^32 means GPIO32

String walletBalanceText = "";
String paymentDetails = "";

String qrData;
uint8_t *framebuffer;

int walletBalance = 0;
uint16_t walletBalanceTextHeight;
int qrSideSize;

void setup()
{
    Serial.begin(115200);
    Serial.println("Lightning Piggy version 1.2.0 starting up");

    // turn on the green LED-IO12 on the PCB
    // otherwise there's no indication that the board is on when it's running on battery power
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    Serial.println("CPU0 reset reason:");
    print_reset_reason(rtc_get_reset_reason(0));
    verbose_print_reset_reason(rtc_get_reset_reason(0));

    Serial.println("CPU1 reset reason:");
    print_reset_reason(rtc_get_reset_reason(1));
    verbose_print_reset_reason(rtc_get_reset_reason(1));

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

    /*
    display.setFont(&Lato_Medium_20);
    String str = "Connecting to WiFi...";
    int16_t x1, y1, cursor_x, cursor_y, box_x, box_y;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    cursor_x = displayWidth() / 2 - w / 2;
    cursor_y = displayHeight() / 2 - h / 2;
    box_x = cursor_x;
    box_y = cursor_y - h;
    //Serial.println(String(box_x) + "," + String(box_y) + "," +String(cursor_x) + "," +String(cursor_y) + "," + String(w) + "," +String(h));
    display.setCursor(cursor_x, cursor_y);
    display.print(str);
    displayBorder();
    display.updateWindow(box_x,box_y,w,h,true);
    */

    Serial.println("Connecting to " + String(ssid));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // bitcoin logo to show wifi is connected
    display.drawBitmap(epd_bitmap_Bitcoin, displayWidth() - 104, 0, 104, 104, GxEPD_WHITE);
    display.updateWindow(displayWidth() - 104, 0, 104, 104, true);


    /*
    str = "Fetching info...";
    display.fillRect(box_x, box_y, w, h, GxEPD_WHITE);
    display.setCursor(cursor_x, cursor_y);
    display.print(str);
    display.updateWindow(box_x,box_y,w,h,true);
    Serial.println("Fetching info should be visible now...");
    delay(10000);
    */
}


void loop() {
    getWalletDetails();

    display.fillScreen(GxEPD_WHITE);

    displayVoltageAndLowBatteryWarning();
    displayBorder();
    printBalance();
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false); // seems needed to avoid artifacts later on when doing partial draws
    display.update();

    getLNURLp();
    showLNURLpQR();
    getLNURLPayments(3);
    display.update();

    hibernate(6 * 60 * 60);
}


int displayHeight() {
  // lilygo 2.66: 152
  return 104; // lilygo 2.13
}

int displayWidth() {
  // lilygo 2.66: 296
  return 212; // lilygo 2.13
}

void displayBorder() {
  horizontalLine();
  verticalLine();
}

void horizontalLine() {
    // Line for showing end of display
    for (int16_t x = 0; x<displayWidth(); x++) {
      display.drawPixel(x,displayHeight()+1,0);
    }
}
void verticalLine() {
    // Line for showing end of display
    for (int16_t y = 0; y<displayHeight(); y++) {
      display.drawPixel(displayWidth()+1,y,0);
    }
}

void printBalance() {
  Serial.println("Displaying balance...");
  int16_t x1, y1;
    uint16_t w, h;
    display.setFont(&Lato_Medium_26);
    display.getTextBounds(walletBalanceText, 0, 0, &x1, &y1, &w, &walletBalanceTextHeight);
    Serial.println("Got text bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(walletBalanceTextHeight)); // typical value for Lato_Medium_26: 1,-19,118,20
    display.setCursor(1, walletBalanceTextHeight);
    display.print(walletBalanceText);
    /* add battery level for testing:
    int batteryLevel = analogRead(35);
    Serial.println("Got battery level: " + String(batteryLevel));
    display.print(walletBalanceText + " B:" + String(batteryLevel));
    */
}


void getWalletDetails() {
  Serial.println("Getting wallet details...");
  const String url = "/api/v1/wallet";
  const String line = getEndpointData(url);
  DynamicJsonDocument doc(4096); // 4096 bytes is plenty for just the wallet details (id, name and balance info)

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  String walletName = doc["name"];

  if (walletName == "null") {
    Serial.println("ERROR: could not find wallet details on lnbits host " + String(host) + " with invoice/read key " + String(invoiceKey) + " so something's wrong! Did you make a typo?");
  }

  walletBalance = doc["balance"];
  walletBalance = walletBalance / 1000;

  Serial.println(walletName);
  Serial.println(String(walletBalance) + " sats");
  walletBalanceText = String(walletBalance) + " sats";
}

/**
 * @brief Get recent LNURL Payments
 * 
 * @param limit 
 */
void getLNURLPayments(int limit) {
  Serial.println("Getting " + String(limit) + " LNURL payments...");
  const uint8_t maxPaymentDetailStrLength = 33; // The maximum number of chars that should be displayed for each payment for Lato_Medium_12 in full width on 212 pixels.
  const String url = "/api/v1/payments?limit=" + String(limit);
  const String line = getEndpointData(url);
  DynamicJsonDocument doc(limit * 4096); // 4KB per lnurlpayment should be enough
  Serial.println("Got payments");
  Serial.println(line);

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  Serial.println("Displaying payment amounts and comments...");
  uint16_t yPos = displayHeight();
  int maxpixels = displayWidth();
  unsigned int maxLinesPerComment = 2;
  String output;
  for (JsonObject areaElems : doc.as<JsonArray>()) {
    if(areaElems["extra"] && !areaElems["pending"] && areaElems["extra"]["tag"]) {
      const char* tag = areaElems["extra"]["tag"];
      if(strcmp(tag,"lnurlp") == 0) {
        int amount = areaElems["amount"];
        amount = amount / 1000; // millisats to sats
        const char* comment = areaElems["extra"]["comment"];
        if (!comment) { // comments can also be a list
          comment = areaElems["extra"]["comment"][0];
        }

        String paymentDetail(comment);
        String paymentAmount(amount);
        paymentDetail = String(paymentAmount) + " sats: " + paymentDetail;
        // first cut off max total length
        paymentDetail = paymentDetail.substring(0, maxPaymentDetailStrLength * maxLinesPerComment);

        // first calculate how many lines are needed
        unsigned int linesNeeded = (paymentDetail.length() / maxPaymentDetailStrLength) + 1;
        linesNeeded = min(maxLinesPerComment, linesNeeded);
        Serial.println("linesNeeded = " + String(linesNeeded));

        // cycle backwards through the lines to show, from the bottom to screen up
        for (int line=linesNeeded-1;line>=0;line--) {
          output = paymentDetail.substring(line*maxPaymentDetailStrLength, line*maxPaymentDetailStrLength+maxPaymentDetailStrLength);
          Serial.println("getLNURLPayments output for line " + String(line) +" = " + output);
          int16_t x1, y1;
          uint16_t w, h;
          display.setFont(&Lato_Medium_12);
          Serial.println("setting cursor to " + String(yPos));
          display.getTextBounds(output, 0, 0, &x1, &y1, &w, &h);
          display.setCursor(0, yPos);
          display.print(output);

          yPos = yPos - h - 1;
        }
      }
    }
  }
}

/**
 * @brief Get the first available LNURLp from the wallet
 *
 */
void getLNURLp() {
  Serial.println("Getting LNURLp link list...");
  // Get the first lnurlp
  String lnurlpData = getEndpointData("/lnurlp/api/v1/links");
  DynamicJsonDocument doc(8192); // we don't know the size of the list of links for this wallet so don't skimp here

  DeserializationError error = deserializeJson(doc, lnurlpData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }
  String lnurlpId = doc[0]["id"];

  Serial.println("Getting LNURLp link for LNURLp ID: " + lnurlpId);
  lnurlpData = getEndpointData("/lnurlp/api/v1/links/" + lnurlpId);
  DynamicJsonDocument firstlink(8192); // create new DynamicJsonDocument as recommended by the docs
  error = deserializeJson(firstlink, lnurlpData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }
  String lnurlp = firstlink["lnurl"];
  qrData = lnurlp;
  Serial.println(lnurlp);
  if (lnurlp == "null") {
    Serial.println("Warning, could not find lnurlp link for this wallet, did you create one?");
    Serial.println("You can do so by activating the LNURLp extension in LNBits, clicking on the extension, and clicking 'NEW PAY LINK'");
    Serial.println("You probably don't want to go for 'fixed amount', but rather for any amount.");
  }
}

/**
 * @brief GET data from an LNbits endpoint
 * 
 * @param endpointUrl 
 * @return String 
 */
String getEndpointData(String endpointUrl) {
  Serial.println("Fetching URL: " + endpointUrl);
  WiFiClientSecure client;
  client.setInsecure(); // see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/README.md

  if (!client.connect(host, 443))
  {
    Serial.println("Server down");
     display.setFont(&Lato_Medium_18);
    printTextCentered((char*)String("I couldn't talk to the Internet :(").c_str());
    hibernate(30 * 60);
  }

  const String request = String("GET ") + endpointUrl + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: LightningPiggy\r\n" +
               "X-Api-Key: " + invoiceKey + " \r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n";

  client.print(request);

  int chunked = 0;
  String line = "";
  while (client.connected())
  {
    line = client.readStringUntil('\n');
    line.toLowerCase();
    if (line == "\r")
    {
      break;
    } else if (line == "transfer-encoding: chunked\r") {
      Serial.println("HTTP chunking enabled");
      chunked = 1;
    }
  }

  if (chunked == 0) {
    line = client.readString();
    return line;
  } else {
    // chunked means first length, then content, then length, then content, until length == "0"
    // no need to support content that has newlines, as it's json so newlines are encoded as \n
    String reply = "";
    line = client.readStringUntil('\n');
    Serial.println("chunked reader got line: " + line);
    while (line != "0\r") {
      reply = reply + client.readStringUntil('\n');
      Serial.println("chunked total reply = " + reply);
      line = client.readStringUntil('\n');
      Serial.println("chunked reader got line: " + line);
    }
    return reply;
  }
}

void printTextCentered(char* str) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth() / 2 - w / 2,
                    displayHeight() / 2 - h / 2 );
    display.print(str);
}

void printTextCenteredX(String str, uint16_t yPos) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth() / 2 - w / 2, yPos );
    display.print(str);
}
