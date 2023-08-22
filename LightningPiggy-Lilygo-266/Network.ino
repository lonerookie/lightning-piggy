#include <WiFiClientSecure.h>

#define BUFFSIZE 256

void connectWifi() {
  Serial.println("Connecting to " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());
}

bool wifiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

// Take measurements of the Wi-Fi strength and return the average result.
// 100 measurements takes 2 seconds so 20ms per measurement
int getStrength(int points){
    long rssi = 0;
    long averageRSSI = 0;

    for (int i=0;i < points;i++){
        rssi += WiFi.RSSI();
        delay(20);
    }

    averageRSSI = rssi/points;
    return averageRSSI;
}

/*
  RSSI Value Range WiFi Signal Strength:
  ======================================
  RSSI > -30 dBm  Amazing
  RSSI < – 55 dBm   Very good signal
  RSSI < – 67 dBm  Fairly Good
  RSSI < – 70 dBm  Okay
  RSSI < – 80 dBm  Not good
  RSSI < – 90 dBm  Extremely weak signal (unusable)

 */
int strengthPercent(float strength) {
  // ESP32 returns RSSI above 0 somethings but not when doing 10 reads...
  // 0 = 100% and -90 = 0%
  //return (90 + strength) * 1.09;
  return 100 + strength;
}

/**
 * @brief GET data from a HTTPS URL
 *
 * @param endpointUrl 
 * @return String 
 */
String getEndpointData(const char * host, String endpointUrl) {
  Serial.println("Fetching URL: " + endpointUrl);
  WiFiClientSecure client;
  client.setInsecure(); // see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/README.md

  if (!client.connect(host, 443))
  {
    Serial.println("Server down");
    setFont(2);
    printTextCentered((char*)String("No internet :-(").c_str());
    hibernate(30 * 60);
  }

  const String request = String("GET ") + endpointUrl + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " + getFullVersion() + "\r\n" +
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
    String reply = "";

    String lengthline = client.readStringUntil('\n');
    Serial.println("chunked reader got length line: '" + lengthline + "'");

    while (lengthline != "0\r") {
      const char *lengthLineChar = lengthline.c_str();
      int bytesToRead = strtol(lengthLineChar, NULL, 16);
      Serial.println("bytesToRead = " + String(bytesToRead));

      int bytesRead = 0;
      while (bytesRead < bytesToRead) { // stop if less than max bytes are read
        uint8_t buff[BUFFSIZE] = {0}; // zero initialize buffer to have 0x00 at the end
        int readNow = min(bytesToRead - bytesRead,BUFFSIZE-1); // leave one byte for the 0x00 at the end
        //Serial.println("Reading bytes: " + String(readNow));
        bytesRead += client.read(buff, readNow);;
        //Serial.print("Got char: "); Serial.write(c);
        String stringBuff = (char*)buff;
        reply += stringBuff;
        //Serial.println("chunked total reply = '" + reply + "'");
      }

      // skip \r\n
      client.read();
      client.read();

      // next chunk length
      lengthline = client.readStringUntil('\n');
      Serial.println("chunked reader got length line: '" + lengthline + "'");
    }

    //Serial.println("returning total chunked reply = '" + reply + "'");
    return reply;
  }
}
