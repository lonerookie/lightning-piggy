#include <ArduinoJson.h>
#include "Constants.h"

float getBitcoinPrice() {
  Serial.println("Getting Bitcoin price...");

  #ifdef DEBUG
  Serial.println("Mocking getBitcoinPrice:"); return 30000.2;
  #endif

  // Get the data
  String path = "/v1/bpi/currentprice/BTC.json";
  if (btcPriceCurrency == CURRENCY_DKK) {
    path = "/v1/bpi/currentprice/DKK.json";
  }

  String priceData = getEndpointData("api.coindesk.com", path, false);
  DynamicJsonDocument doc(8192); // the size of the list of links is unknown so don't skimp here

  DeserializationError error = deserializeJson(doc, priceData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return NOT_SPECIFIED;
  }

  Serial.println("Extracting bitcoin price from received data");

  float btcPrice = (float)NOT_SPECIFIED;
  if (btcPriceCurrency == CURRENCY_USD) {
     btcPrice = doc["bpi"]["USD"]["rate_float"];
  }
  else if (btcPriceCurrency == CURRENCY_DKK) {
    btcPrice = doc["bpi"]["DKK"]["rate_float"];
  }

  Serial.print("BTC Price: ");
  Serial.println(btcPrice);
  return btcPrice;
}