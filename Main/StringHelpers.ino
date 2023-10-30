String formatFloatWithSeparator(float number) 
{
  if (number < 1000) {
    return String(number, 0);
  }

  int numberAsInt = (int)number; 
  Serial.println("numberAsInt" + String(numberAsInt));

  int remainder = numberAsInt % 1000;
  Serial.println("remainder" + String(remainder));
  return String(number / 1000.0f, 0) + getCurrentCurrencyThousandsSeparator() + String(remainder);
}

String getCurrentCurrencyCode() {
  if (btcPriceCurrency == CURRENCY_USD) {
    return "USD";
  }
  else if (btcPriceCurrency == CURRENCY_DKK) {
    return "KR";
  } 
}

char getCurrentCurrencyDecimalSeparator() {
  if (btcPriceCurrency == CURRENCY_USD) {
    return '.';
  }
  else if (btcPriceCurrency == CURRENCY_DKK) {
    return ',';
  } 
}

String getCurrentCurrencyThousandsSeparator() {
  if (btcPriceCurrency == CURRENCY_USD) {
    return ",.";
  }
  else if (btcPriceCurrency == CURRENCY_DKK) {
    return ".";
  } 
}

String floatToString(float number, int decimals) {
  if (btcPriceCurrency == CURRENCY_USD) {
    return String(number, decimals);
  }

  char buffer[15]; 

  dtostrf(number, 6, decimals, buffer); /

  for (int i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '.') {
      buffer[i] = getCurrentCurrencyDecimalSeparator();
      break; 
    }
  }

  return String(buffer); 
}