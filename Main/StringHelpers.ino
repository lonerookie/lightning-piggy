String formatFloatWithSeparator(float number) 
{
  if (number < 1000) {
    return String(number, 0);
  }

  int numberAsInt = (int)number; 
  float thousands = number / 1000.0f;
  int remainder = numberAsInt % 1000;

  return String(thousands, 0) + getCurrentCurrencyThousandsSeparator() + String(remainder);
}

String getCurrentCurrencyCode() {
  switch (btcPriceCurrency) {
    case CURRENCY_USD:
      return "$";
    case CURRENCY_DKK:
      return "KR";
    default:
      return "NA";
  }
}

char getCurrentCurrencyDecimalSeparator() {
  switch (btcPriceCurrency) {
    case CURRENCY_USD:
      return '.';
    case CURRENCY_DKK:
      return ',';
    default:
      return ' ';
  }
}

char getCurrentCurrencyThousandsSeparator() {
  switch (btcPriceCurrency) {
    case CURRENCY_USD:
      return ',';
    case CURRENCY_DKK:
      return '.';
    default:
      return ' ';
  }
}

String floatToString(float number, int decimals) {
  if (btcPriceCurrency == CURRENCY_USD) {
    return String(number, decimals);
  }

  char buffer[15]; 

  dtostrf(number, 6, decimals, buffer);

  for (int i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '.') {
      buffer[i] = getCurrentCurrencyDecimalSeparator();
      break; 
    }
  }

  return String(buffer); 
}
