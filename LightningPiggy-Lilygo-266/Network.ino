#include <WiFiClientSecure.h>

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
    // no need to support content that has newlines, as it's json so newlines are encoded as \n
    String reply = "";

    String lengthline = client.readStringUntil('\n');
    Serial.println("chunked reader got length line: '" + lengthline + "'");

    while (lengthline != "0\r") {
      const char *lengthLineChar = lengthline.c_str();
      int bytesToRead = strtol(lengthLineChar, NULL, 16);
      Serial.print("bytesToRead = ");
      Serial.println(bytesToRead);

      // TODO: speed up this function by reading multiple bytes at a time: int read(uint8_t *buf, size_t size);
      // Currently it takes around 500ms, which is not bad, but not great.
      Serial.print("millis() before slow function: "); Serial.println(millis());
      int bytesRead = 0;
      for (int i=0;i<bytesToRead;i++) {
        char c = client.read();
        //Serial.print("Got char: "); Serial.write(c);
        String byteRead(c);
        reply = reply + byteRead;
        //Serial.println("chunked total reply = '" + reply + "'");
      }
      Serial.print("millis() after slow function: "); Serial.println(millis());

      // first the newline character so skip it:
      lengthline = client.readStringUntil('\n');
      Serial.println("chunked reader got empty line due to newline character: '" + lengthline + "'");

      // then the real length
      lengthline = client.readStringUntil('\n');
      Serial.println("chunked reader got length line: '" + lengthline + "'");
    }

    /*
    reply = reply + client.readBytes(bytesToRead);
    while (line != "0\r") {
      reply = reply + client.readStringUntil('\n');
      Serial.println("chunked total reply = '" + reply + "'");
      line = client.readStringUntil('\n');
      Serial.println("chunked reader got length line: '" + line + "'");
    }
    */
    Serial.println("returning total chunked reply = '" + reply + "'");
    return reply;
  }
}
