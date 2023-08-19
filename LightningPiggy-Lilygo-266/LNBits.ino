#include <ArduinoJson.h>

int getWalletBalance() {
  Serial.println("Getting wallet details...");
  const String url = "/api/v1/wallet";

  #ifdef DEBUG
  return 12345678;
  #endif

  const String line = getEndpointData(lnbitsHost, url);
  DynamicJsonDocument doc(4096); // 4096 bytes is plenty for just the wallet details (id, name and balance info)

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  String walletName = doc["name"];

  if (walletName == "null") {
    Serial.println("ERROR: could not find wallet details on lnbits host " + String(lnbitsHost) + " with invoice/read key " + String(invoiceKey) + " so something's wrong! Did you make a typo?");
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
void getLNURLPayments(int limit, int maxX, int startY) {
  Serial.println("Getting " + String(limit) + " LNURL payments...");

  uint8_t maxPaymentDetailStrLength = 20; // Leave some space on the right for other values
  #ifdef LILYGO_T5_V266
  maxPaymentDetailStrLength += 5;
  #endif

  const String url = "/api/v1/payments?limit=" + String(limit);

  #ifdef DEBUG
  const String line = "[{\"checking_id\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690630482,\"bolt11\":\"lnbc10n1pjvf72jsp5w99r5sg4kqnhjl2syltkwxf2gm86p0p3mh2rm5fwahxmwr047l3spp5atshpyn7rccgz89jg2j8gdhvg6hln778xsys088r0ulg3z75taashp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqdjs3alg9wmchtfs0nav7nkls58u8usv5pc742q8fkw4gf9fpykqkzahvuqq2sgqqyqqqqqqqqqqqeqqjq9qxpqysgqttwctdvcq64s5tv0qemcykhw4atv7l3nta0029z75ve35xxk03mp6q2cs5yznnwp0euchrq7tw8heg98p7xylq9cl5jmd45r55zttfcp83pzun\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"expiry\":1690631082.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"This is a very long message, so let's see how it shows up. Woohoow! Looking great!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690622074,\"bolt11\":\"lnbc10n1pjvfkr6sp5rxqjqmufhp48etlsgxepwp33k2scctap3vvh63py3c5wtlvk93pspp56hp6sutk5z5f4vd3u35ua3xscnt5w6q8g2mncdammkdclkqa3myshp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjq0geslmtzh3zmklrmwe4v8l5fqv52y4wjs87nx9m9efxj74xrehh7rqafqqqwkqqqyqqqqlgqqqqqqgq2q9qxpqysgqflaf37unptvtzs738xalks6fz7xkh5jn0hem5xzmkgcznpcfa8fk6wtulumxmvpu7dzj440j20mvqjqjhhsr3y6td9asz7wlnh4080gq93wz75\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"expiry\":1690622674.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"hello there! Here's some sats!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null}]";
  #else
  const String line = getEndpointData(lnbitsHost, url);
  #endif

  Serial.println("Got payments");
  Serial.println(line);

  DynamicJsonDocument doc(limit * 4096); // 4KB per lnurlpayment should be enough for everyone (tm)
  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  Serial.println("Displaying payment amounts and comments...");
  uint16_t yPos = startY;
  int maxpixels = maxX;
  unsigned int maxLinesPerComment = 3;
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
        String units = "sats";
        if (amount < 2) units = "sat";
        paymentDetail = paymentAmount + " " + units + ": " + paymentDetail;

        // first calculate how many lines are needed
        unsigned int linesNeeded = (paymentDetail.length() / maxPaymentDetailStrLength) + 1;
        Serial.println("linesNeeded = " + String(linesNeeded));
        linesNeeded = min(maxLinesPerComment, linesNeeded);
        Serial.println("linesNeeded = " + String(linesNeeded));

        // cycle through the lines to show
        for (int line=0;line<linesNeeded;line++) {
          output = paymentDetail.substring(line*maxPaymentDetailStrLength, line*maxPaymentDetailStrLength+maxPaymentDetailStrLength);
          Serial.println("getLNURLPayments output for line " + String(line) +" = " + output);
          int16_t x1, y1;
          uint16_t w, h;
          setFont(2);
          Serial.println("setting cursor to " + String(yPos));
          display.getTextBounds(output, 0, 0, &x1, &y1, &w, &h);
          Serial.println("getTextBounds: " + String(x1) + "," + String(y1) + ","+ String(w) + ","+ String(h));
          display.setCursor(0, yPos + h);
          display.print(output);

          yPos = yPos + h + 1;
        }
        yPos += 4;
        #ifdef LILYGO_T5_V266
        yPos += 4;
        #endif
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

  #ifdef DEBUG
  Serial.println("Mocking getLNURLp:"); return "LNURL1DP68GURN8GHJ7MR9VAJKUEPWD3HXY6T5WVHXXMMD9AKXUATJD3CZ7DTRWE2NVKQ72L5D3";
  #endif

  // Get the first lnurlp
  String lnurlpData = getEndpointData(lnbitsHost, "/lnurlp/api/v1/links");
  DynamicJsonDocument doc(8192); // we don't know the size of the list of links for this wallet so don't skimp here

  DeserializationError error = deserializeJson(doc, lnurlpData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }
  String lnurlpId = doc[0]["id"];
  String lnurlp = doc[0]["lnurl"];

  Serial.println(lnurlp);
  return lnurlp;
}
