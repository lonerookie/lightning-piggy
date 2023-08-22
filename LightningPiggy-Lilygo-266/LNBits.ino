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
  } else {
    Serial.print("Wallet name: " + walletName);
  }

  int walletBalance = doc["balance"];
  walletBalance = walletBalance / 1000;

  Serial.println(" contains " + String(walletBalance) + " sats");
  return walletBalance;
}

/**
 * @brief Get recent LNURL Payments
 * 
 * @param limit 
 */
void getLNURLPayments(int limit, int maxX, int startY) {

  // Draw a line under the total sats amount
  display.fillRect(0, startY+2, maxX-3, 1, GxEPD_BLACK);
  startY+=4;

  Serial.println("Getting " + String(limit) + " LNURL payments...");

  const String url = "/api/v1/payments?limit=" + String(limit);

  #ifdef DEBUG
  // To escape quotes and newlines, use:
  // sed 's/"/\\"/g' | sed "s/\\\n/\\\\\\\n/g"
  // regular testcase with long comment:
  const String line = "[{\"checking_id\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690630482,\"bolt11\":\"lnbc10n1pjvf72jsp5w99r5sg4kqnhjl2syltkwxf2gm86p0p3mh2rm5fwahxmwr047l3spp5atshpyn7rccgz89jg2j8gdhvg6hln778xsys088r0ulg3z75taashp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqdjs3alg9wmchtfs0nav7nkls58u8usv5pc742q8fkw4gf9fpykqkzahvuqq2sgqqyqqqqqqqqqqqeqqjq9qxpqysgqttwctdvcq64s5tv0qemcykhw4atv7l3nta0029z75ve35xxk03mp6q2cs5yznnwp0euchrq7tw8heg98p7xylq9cl5jmd45r55zttfcp83pzun\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"expiry\":1690631082.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"This is a very long message, so let's see how it shows up. Woohoow! Looking great!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690622074,\"bolt11\":\"lnbc10n1pjvfkr6sp5rxqjqmufhp48etlsgxepwp33k2scctap3vvh63py3c5wtlvk93pspp56hp6sutk5z5f4vd3u35ua3xscnt5w6q8g2mncdammkdclkqa3myshp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjq0geslmtzh3zmklrmwe4v8l5fqv52y4wjs87nx9m9efxj74xrehh7rqafqqqwkqqqyqqqqlgqqqqqqgq2q9qxpqysgqflaf37unptvtzs738xalks6fz7xkh5jn0hem5xzmkgcznpcfa8fk6wtulumxmvpu7dzj440j20mvqjqjhhsr3y6td9asz7wlnh4080gq93wz75\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"expiry\":1690622674.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"hello there! Here's some sats!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null}]";
  // regular testcase with very long comment:
  // const String line = "[{\"checking_id\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690630482,\"bolt11\":\"lnbc10n1pjvf72jsp5w99r5sg4kqnhjl2syltkwxf2gm86p0p3mh2rm5fwahxmwr047l3spp5atshpyn7rccgz89jg2j8gdhvg6hln778xsys088r0ulg3z75taashp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqdjs3alg9wmchtfs0nav7nkls58u8usv5pc742q8fkw4gf9fpykqkzahvuqq2sgqqyqqqqqqqqqqqeqqjq9qxpqysgqttwctdvcq64s5tv0qemcykhw4atv7l3nta0029z75ve35xxk03mp6q2cs5yznnwp0euchrq7tw8heg98p7xylq9cl5jmd45r55zttfcp83pzun\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"expiry\":1690631082.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"This is a very long message, so let's see how it shows up. Woohoow! Looking great! BUT what if it was REALLY way too long to show, making the other text disappear? What will happen? Do we still show two messages? Let's find out!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690622074,\"bolt11\":\"lnbc10n1pjvfkr6sp5rxqjqmufhp48etlsgxepwp33k2scctap3vvh63py3c5wtlvk93pspp56hp6sutk5z5f4vd3u35ua3xscnt5w6q8g2mncdammkdclkqa3myshp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjq0geslmtzh3zmklrmwe4v8l5fqv52y4wjs87nx9m9efxj74xrehh7rqafqqqwkqqqyqqqqlgqqqqqqgq2q9qxpqysgqflaf37unptvtzs738xalks6fz7xkh5jn0hem5xzmkgcznpcfa8fk6wtulumxmvpu7dzj440j20mvqjqjhhsr3y6td9asz7wlnh4080gq93wz75\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"expiry\":1690622674.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"hello there! Here's some sats!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null}]";
  // regular testcase with extremely long comment:
  // const String line = "[{\"checking_id\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690630482,\"bolt11\":\"lnbc10n1pjvf72jsp5w99r5sg4kqnhjl2syltkwxf2gm86p0p3mh2rm5fwahxmwr047l3spp5atshpyn7rccgz89jg2j8gdhvg6hln778xsys088r0ulg3z75taashp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqdjs3alg9wmchtfs0nav7nkls58u8usv5pc742q8fkw4gf9fpykqkzahvuqq2sgqqyqqqqqqqqqqqeqqjq9qxpqysgqttwctdvcq64s5tv0qemcykhw4atv7l3nta0029z75ve35xxk03mp6q2cs5yznnwp0euchrq7tw8heg98p7xylq9cl5jmd45r55zttfcp83pzun\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"eae170927e1e30811cb242a47436ec46aff9fbc73409079ce37f3e888bd45f7b\",\"expiry\":1690631082.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"This is a very long message, so let's see how it shows up. Woohoow! Looking great! BUT what if it was REALLY way too long to show, making the other text disappear? What will happen? Do we still show two messages? Let's find out! WOW it is really capable of showing huge texts now, with this tiny font. So it needs to be even longer. And longer. Much longer! Now we are really pushing it beyond the limit of even the smallest font and the biggest display. So now something has got to give!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1690622074,\"bolt11\":\"lnbc10n1pjvfkr6sp5rxqjqmufhp48etlsgxepwp33k2scctap3vvh63py3c5wtlvk93pspp56hp6sutk5z5f4vd3u35ua3xscnt5w6q8g2mncdammkdclkqa3myshp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjq0geslmtzh3zmklrmwe4v8l5fqv52y4wjs87nx9m9efxj74xrehh7rqafqqqwkqqqyqqqqlgqqqqqqgq2q9qxpqysgqflaf37unptvtzs738xalks6fz7xkh5jn0hem5xzmkgcznpcfa8fk6wtulumxmvpu7dzj440j20mvqjqjhhsr3y6td9asz7wlnh4080gq93wz75\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"d5c3a87176a0a89ab1b1e469cec4d0c4d747680742b73c37bbdd9b8fd81d8ec9\",\"expiry\":1690622674.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"hello there! Here's some sats!\"],\"wh_status\":404,\"wh_success\":false,\"wh_message\":\"Not Found\",\"wh_response\":\"response\"},\"wallet_id\":\"12345678901234567890123456789012\",\"webhook\":null,\"webhook_status\":null}]";
  // bitcoin3us' test case modified for 99999999 sats, one without comment, one with comment, both without webhooks:
  // const String line = "[{\"checking_id\":\"341e56153a2896b03ac653bdca88818bcc620be54d98faf1e56456e2c6caccdd\",\"pending\":false,\"amount\":99999999000,\"fee\":0,\"memo\":\"tip\",\"time\":1691438661,\"bolt11\":\"lnbc4140n1pjdznj9sp5ddzgugjfpqe9wpxu379rgn3sn6cpl2fnqk3h0wzjwga47rh5uwzqpp5xs09v9f69zttqwkx2w7u4zyp30xxyzl9fkv04u09v3tw93k2enwshp5z2jnl04hmynfk73xzsnrpm4dykcjrt6zf5lhk9rhfqg223q798xsxqzjccqpjrzjqf0wqzupw7dv2pt4f4fl4c2xkw6h0ctg8j95ax0vp3k0h38n7a7sgrpvq5qqzwgqqyqqqqlgqqqqp9sq2q9qxpqysgqex2w0mkcku6w80tc3l3uhvf076qq62sdk7sp374g6etvgnhrs4nkg73gecz4ed57677ccdhht9e8ej997vk27298p4gcjs40tfsz6cgpzn8uh2\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"341e56153a2896b03ac653bdca88818bcc620be54d98faf1e56456e2c6caccdd\",\"expiry\":1691439261.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"MB95To\",\"extra\":\"414000\"},\"wallet_id\":\"e5e8ce89a00a4dfeb461d6875888a4b2\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"af3bbf8463c6e5a444232f78f14138fb49bfdb096f448e70f5ff0f4878d452bb\",\"pending\":false,\"amount\":101000,\"fee\":0,\"memo\":\"tip\",\"time\":1689171770,\"bolt11\":\"lnbc1010n1pj2ade6sp54svuufvuwhzsp3c5fvxq0r0r7gzz4ggurvc3w8zmq9e6km7x2hvspp54uamlprrcmj6g3pr9au0zsfcldymlkcfdazguu84lu85s7x522ashp5z2jnl04hmynfk73xzsnrpm4dykcjrt6zf5lhk9rhfqg223q798xsxqzjccqpjrzjq2tt9ke59l8c0655mxqh2l7lf5l9gk74em6fr86ckhfcmlwh806ujz6cnsqqt5cqqqqqqqlgqqqcdgqq9q9qyysgqdrmqjx6x577dc8spylnuqefla4l4dtus2ksn3c9nmhx934wmsx9rkqpunzs9pp976mqj94v45x7xegp4jrsyp7efjwhas4e525lgqhqpgpkqze\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"af3bbf8463c6e5a444232f78f14138fb49bfdb096f448e70f5ff0f4878d452bb\",\"expiry\":1689172370.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"MB95To\",\"extra\":\"101000\",\"comment\":[\"Well done piggy!!\"]},\"wallet_id\":\"e5e8ce89a00a4dfeb461d6875888a4b2\",\"webhook\":null,\"webhook_status\":null}]";
  // one with comment, one without comment, both with webhook:
  // const String line = "[{\"checking_id\":\"30f89c969d07b1e9851911320e16dca655b2805aac5091978091228f7dc80dc0\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1692453842,\"bolt11\":\"lnbc10n1pjwpj7jsp5ng25uynapv0zggf3en87gx07nmd0fa3prp7d0ms7mdhgw7yvjp5spp5xrufe95aq7c7npgezyequ9ku5e2m9qz643gfr9uqjy3g7lwgphqqhp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqf0wqzupw7dv2pt4f4fl4c2xkw6h0ctg8j95ax0vp3k0h38n7a7sgrpvq5qqzwgqqyqqqqlgqqqqp9sq2q9qxpqysgqzuzj2dctaz4knqqq32tjzdn943p5zphtexk04mhnyx2v2ysyazxq59vmdelsh79k8jtd7wez09y4zv33zlehlfj9jpperf9d8tnmm0gqrjrdhz\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"30f89c969d07b1e9851911320e16dca655b2805aac5091978091228f7dc80dc0\",\"expiry\":1692454442.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"comment\":[\"Long comments are great for testing! BIG ones and small ones. Let's see how it gets displayed! Even if it is super long, it should still somehow show up.\"],\"wh_status\":200,\"wh_success\":true,\"wh_message\":\"OK\",\"wh_response\":\"POK\"},\"wallet_id\":\"5d59feaf97554bcd872c2bd686891d9e\",\"webhook\":null,\"webhook_status\":null},{\"checking_id\":\"8d8f9b6c959ffab9a1cb5f6505a307c5e1df0e4e880b260af1109817a03ea3b4\",\"pending\":false,\"amount\":1000,\"fee\":0,\"memo\":\"piggytest\",\"time\":1692456573,\"bolt11\":\"lnbc10n1pjwp4nasp5wdc2c3xh2ytvamgnnl82ujywg7v4gvk86yymzzep24kx3wp03mgqpp53k8ekmy4nlatngwttajstgc8chsa7rjw3q9jvzh3zzvp0gp75w6qhp5gxwncgtpe3jmwprje9eyysh7ap0xe2ez8uy59s436xftc9vd0cdqxqzjccqpjrzjqtxlgn2nt3cqqlmej5nqc89ua89d6hdrrtc8dvslgtc4agauknv2wzlytyqqy4gqqyqqqqqqqqqqxxcpyq9qxpqysgqqlmez48jpy88tepv6dxj9mqpv8he0vwg93md29y4g8neqs8cta7k38e3e43hdlrjsfd5lf4axp03fahkry7w9wtrp0cy43udqvrdepqqt4u9xg\",\"preimage\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"payment_hash\":\"8d8f9b6c959ffab9a1cb5f6505a307c5e1df0e4e880b260af1109817a03ea3b4\",\"expiry\":1692457173.0,\"extra\":{\"tag\":\"lnurlp\",\"link\":\"5cvU6X\",\"extra\":\"1000\",\"wh_status\":200,\"wh_success\":true,\"wh_message\":\"OK\",\"wh_response\":\"POK\"},\"wallet_id\":\"5d59feaf97554bcd872c2bd686891d9e\",\"webhook\":null,\"webhook_status\":null}]";
  #else
  const String line = getEndpointData(lnbitsHost, url);
  #endif

  Serial.println("Got payments: " + line);

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
  for (JsonObject areaElems : doc.as<JsonArray>()) {
    Serial.println("Parsing payment...");
    if(areaElems["extra"] && !areaElems["pending"] && areaElems["extra"]["tag"]) {

      // Only do lnurlp payments
      const char* tag = areaElems["extra"]["tag"];
      if(strcmp(tag,"lnurlp") == 0) {

        // Payment always has an amount
        long long amount = areaElems["amount"]; // long long to support amounts above 999999000 millisats
        long amountSmaller = amount / 1000; // millisats to sats
        String paymentAmount(amountSmaller);
        String units = "sats";
        if (amountSmaller < 2) units = "sat";
        String paymentDetail = paymentAmount + " " + units;

        // Payment has an optional comment
        if (areaElems["extra"]["comment"]) {
          Serial.println("Getting comment...");
          const char* comment = areaElems["extra"]["comment"];
          if (!comment && areaElems["extra"]["comment"][0]) { // comments can also be a list
            //Serial.println("Getting comment from list...");
            comment = areaElems["extra"]["comment"][0];
          }
          String paymentComment(comment);
          paymentDetail += ": " + paymentComment;
        } else {
          paymentDetail += "!"; // no comment so "99999999 sats!" (= almost 1 BTC) will fit on one line in big font
        }

        // Display the message
        int fontSize = 3; // first try the biggest font, then go smaller
        uint16_t savedYPos = yPos; // save yPos so it can be restored

        // If not enough space is available (because the first message took it all) then claw it back
        if (savedYPos > displayHeight() * 0.8) {
          Serial.println("Not enough vertical space is provided, taking it...");
          savedYPos = displayHeight() * 0.8;
          Serial.println("savedYPos = " + String(savedYPos));
        }

        while (fontSize > 0) {
          setFont(fontSize);
          yPos = savedYPos;
          display.fillRect(0, yPos, maxX, displayHeight(), GxEPD_WHITE);

          // display the entire text
          int textPos = 0;
          while (textPos < paymentDetail.length()) {
            // Try to fit everything that still needs displaying:
            String paymentDetailWithoutAlreadyPrintedPart = paymentDetail.substring(textPos);
            int chars = fitMaxText(paymentDetailWithoutAlreadyPrintedPart, maxX);

            // Print the text that fits:
            String textLine = paymentDetail.substring(textPos, textPos+chars);
            //Serial.println("first line that fits: " + textLine);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(textLine, 0, 0, &x1, &y1, &w, &h);
            //Serial.println("getTextBounds of textLine: " + String(x1) + "," + String(y1) + ","+ String(w) + ","+ String(h));
            display.setCursor(0, yPos + h); // bottom of the line
            display.print(textLine);

            textPos += chars;
            yPos += h + 1;
          }
          //Serial.println("After writing the paymentDetail, yPos = " + String(yPos) + " while displayHeight = " + String(displayHeight()));

          // Check if the entire text fit:
          if (yPos < displayHeight()) {
            Serial.println("yPos < displayHeight so fontSize " + String(fontSize) + " fits!");
            break; // exit the fontSize loop because it fits
          } else {
            Serial.println("fontSize " + String(fontSize) + " did not fit so trying smaller...");
            fontSize--;
          }
        }
        //Serial.println("After fontSize loop, yPos = " + String(yPos));

        yPos += 2;
      } else {
        Serial.println("Skipping because extra tag is not lnurlp...");
      }
    } else {
      Serial.println("Skipping because no extra or no extra tag or still pending...");
    }
  }
  display.updateWindow(0, startY, maxX, displayHeight()-startY, true);
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
  DynamicJsonDocument doc(8192); // the size of the list of links is unknown so don't skimp here

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
