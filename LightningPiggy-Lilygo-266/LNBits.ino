#include <ArduinoJson.h>

int getWalletBalance() {
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

  int walletBalance = doc["balance"];
  walletBalance = walletBalance / 1000;

  Serial.println(walletName);
  Serial.println(String(walletBalance) + " sats");

  return walletBalance;
}

/**
 * @brief Get recent LNURL Payments
 * 
 * @param limit 
 */
void getLNURLPayments(int limit) {
  Serial.println("Getting " + String(limit) + " LNURL payments...");

  //const uint8_t maxPaymentDetailStrLength = 33; // The maximum number of chars that should be displayed for each payment for Lato_Medium_12 in full width on 212 pixels.
  const uint8_t maxPaymentDetailStrLength = 26; // Leave some space on the right for other values

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
          setFont(1);
          Serial.println("setting cursor to " + String(yPos));
          display.getTextBounds(output, 0, 0, &x1, &y1, &w, &h);
          //Serial.println("getTextBounds: " + String(x1) + "," + String(y1) + ","+ String(w) + ","+ String(h));
          display.setCursor(0, yPos);
          display.print(output);

          yPos = yPos - h - 2;
        }
      }
    }
  }
}

/**
 * @brief Get the first available LNURLp from the wallet
 *
 * @return lnurlp for accepting payments
 *
 */
String getLNURLp() {
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
  Serial.println(lnurlp);
  return lnurlp;
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
    setFont(2);
    printTextCentered((char*)String("No internet :-(").c_str());
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
