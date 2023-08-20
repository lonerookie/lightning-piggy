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
