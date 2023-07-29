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
#include <ArduinoJson.h>

#define LILYGO_T5_V266
#include <GxEPD.h>
#include <boards.h>
#include "qrcoded.h"

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

// These values get replaced with the user provided values by the Web Serial Installer for Lightning Piggy.
// But you can also replace them manually yourself here if you don't plan on using the Web Installer.
const char* ssid     = "REPLACETHISBYWIFISSID_REPLACETHISBYWIFISSID_REPLACETHISBYWIFISSID"; // wifi SSID here
const char* password = "REPLACETHISBYWIFIKEY_REPLACETHISBYWIFIKEY_REPLACETHISBYWIFIKEY"; // wifi password here
const char* host = "REPLACETHISBYLNBITSHOST_REPLACETHISBYLNBITSHOST_REPLACETHISBYLNBITSHOST"; // HOST NAME HERE E.G. legend.lnbits.com
const char* invoiceKey = "REPLACETHISBYLNBITSKEY_REPLACETHISBYLNBITSKEY_REPLACETHISBYLNBITSKEY"; // lnbits wallet invoice hey here

String walletBalanceText = "";
String paymentDetails = "";

String qrData;
uint8_t *framebuffer;

int walletBalance = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Lightning Piggy version 1.0.7 starting up");

    // turn on the green LED-IO12 on the PCB
    // otherwise there's no indication that the board is on when it's running on battery power
    // it will turn off when the board hibernates
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)

    // read and print battery level
    int batteryLevel = analogRead(35);
    Serial.println("Got battery level: " + String(batteryLevel));

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();

    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);

    display.fillScreen(GxEPD_WHITE);
    display.setFont(&Lato_Medium_20);
    printTextCentered("Connecting to WiFi");
    display.update();

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    display.fillScreen(GxEPD_WHITE);
    display.setFont(&Lato_Medium_20);
    printTextCentered("Updating Piggy");
    display.update();

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(100);
}


void loop()
{
  getWalletDetails();

  display.fillScreen(GxEPD_WHITE);
  printBalance();
  getLNURLPayments(3);
  display.update();
  delay(10000);
  getLNURLp();
  showLNURLpQR();
  hibernate(6 * 60 * 60);
}

void printBalance() {
  Serial.println("Displaying balance...");
  int16_t x1, y1;
    uint16_t w, h;
    display.setFont(&Lato_Medium_26);
    display.getTextBounds(walletBalanceText, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2, 40);
    /* print battery level for testing:
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
  const uint8_t maxPaymentDetailStrLength = 30; // The maximum number of chars that should be displayed for each payment
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
  uint16_t yPos = 70;
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

        display.setFont(&Lato_Medium_12);
        display.setCursor(10, yPos);
        String paymentDetail(comment);
        String paymentAmount(amount);
        output = paymentDetail.substring(0, maxPaymentDetailStrLength) + " " + paymentAmount + " sats";
        Serial.println("getLNURLPayments output: " + output);

        printTextCenteredX(output, yPos);
        yPos += 19;
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

void showLNURLpQR() {
  if (qrData == "null") {
    Serial.println("INFO: not showing LNURLp QR code because no LNURLp code was found.");
    return;
  }
  Serial.println("Building LNURLp QR code...");

  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;

  int qrVersion = getQrCodeVersion();
  int pixSize = getQrCodePixelSize(qrVersion);
  uint8_t qrcodeData[qrcode_getBufferSize(qrVersion)];
  
  qrcode_initText(&qrcoded, qrcodeData, qrVersion, 0, qrDataChar);

  Serial.println("Displaying LNURLp QR code...");
  int qrWidth = pixSize * qrcoded.size;
  int qrPosX = ((display.width() - qrWidth) / 2);
  // int qrPosY = ((EPD_HEIGHT - qrWidth) / 2);
  int qrPosY = 10;

  display.fillScreen(GxEPD_WHITE);

  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        display.fillRect(qrPosX + pixSize * x, qrPosY + pixSize * y, pixSize, pixSize, GxEPD_BLACK);
      }
    }
  }
  display.setFont(&Lato_Medium_12);
  printTextCenteredX("Ready to receive satoshis!", display.height() - 10);
  display.update();
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
    printTextCentered("Oink. I couldn't talk to the Internet :(");
    hibernate(30 * 60);
    delay(3000);
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
    while (line != "0\r") {
      reply = reply + client.readStringUntil('\n');
      line = client.readStringUntil('\n');
    }
    return reply;
  }
}

void hibernate(int sleepTimeSeconds) {
  uint64_t deepSleepTime = (uint64_t)sleepTimeSeconds * (uint64_t)1000 * (uint64_t)1000;
  Serial.println("Going to sleep for seconds");
  Serial.println(sleepTimeSeconds);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(deepSleepTime);
  esp_deep_sleep_start();
}


void printTextCentered(char* str) {
  int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2,
                    display.height() / 2 - h / 2 );
    display.print(str);
}

void printTextCenteredX(String str, uint16_t yPos) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2, yPos );
    display.print(str);
}

/**
 * @brief Get the size of the qr code to produce
 * 
 * @param qrData 
 * @return int 
 */
int getQrCodeVersion() {
  int qrVersion = 0;
  int stringLength = qrData.length();

  // Using this chart with ECC_LOW https://github.com/ricmoo/QRCode#data-capacities
  if(stringLength <= 25) {
    qrVersion = 1;
  }  
  else if(stringLength <= 47) {
    qrVersion = 2;
  }
  else if(stringLength <= 77) {
    qrVersion = 3;
  }
  else if(stringLength <= 114) {
    qrVersion = 4;
  }
  else if(stringLength <= 154) {
    qrVersion = 5;
  }
  else if(stringLength <= 195) {
    qrVersion = 6;
  }
  else if(stringLength <= 367) {
    qrVersion = 11;
  }
  else {
    qrVersion = 28;
  }

  return qrVersion;
}


/**
 * @brief Get the Qr Code Pixel Size object
 * 
 * @param qrCodeVersion The QR code version that is being used
 * @return int The size of the QR code pixels
 */
int getQrCodePixelSize(int qrCodeVersion) {
  Serial.println("getQrCodePixelSize for qrCodeVersion " + String(qrCodeVersion));
  
  int qrDisplayHeight = display.height() - 20; // qr code height in pixels
  // Using https://github.com/ricmoo/QRCode#data-capacities

  // Get the QR code size (blocks not pixels)
  int qrCodeHeight = 0;
  switch(qrCodeVersion) {
    case 1:
      qrCodeHeight = 21;
      break;
    case 2:
      qrCodeHeight = 25;
      break;
    case 3:
      qrCodeHeight = 29;
      break;
    case 4:
      qrCodeHeight = 33;
      break;
    case 5:
      qrCodeHeight = 37;
      break;
    case 6:
      qrCodeHeight = 41;
      break;
    case 7:
      qrCodeHeight = 45;
      break;
    case 8:
      qrCodeHeight = 49;
      break;
    case 9:
      qrCodeHeight = 53;
      break;
    case 10:
      qrCodeHeight = 57;
      break;
    case 11:
      qrCodeHeight = 61;
      break;
    default:
      qrCodeHeight = 129;
      break;
  }
  int pixelHeight = floor(qrDisplayHeight / qrCodeHeight);
  Serial.println(F("qrCodeHeight pixel height is"));
  Serial.println(qrCodeHeight);

  Serial.println(F("Calced pixel height is"));
  Serial.println(pixelHeight);
  //return pixelHeight;
  return 2;
}
