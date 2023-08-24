
void setup_display() {
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();

    // partial update to full screen to preset for partial update of box window (this avoids strange background effects)
    // this needs to be done before setRotation, otherwise still faint/missing pixels, even with using_rotation = true
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1); // display is used in landscape mode
    display.update(); // clear the screen from any old data that might faintly be there
}

int displayHeight() {
  // lilygo 2.66 is 152px, lilygo 2.13 is 122px
  #ifdef LILYGO_T5_V213
  return 122; // on the LILYGO_T5_V213, GxEPD_WIDTH is incorrectly set to 128
  #else
  //return 122; // for testing the 2.13's lower resolution on the 2.66's high res display
  return GxEPD_WIDTH; // width and height are swapped because display is rotated
  #endif
}

int displayWidth() {
  // lilygo 2.66 is 296px, lilygo 2.13 is 250px
  //return 250; // for testing the 2.13's lower resolution on the 2.66's high res display
  return GxEPD_HEIGHT; // width and height are swapped because display is rotated
}

// size 0 = smallest font (8pt)
// size 1 = 12pt
// size 2 = 18pt
// size 3 = 20pt
// size 4 = 26pt
void setFont(int fontSize) {
  if (fontSize < 0) {
    Serial.println("ERROR: font size " + String(fontSize) + " is not supported, setting min size");
    display.setFont(&Lato_Medium_8);
  } else if (fontSize == 0) {
    display.setFont(&Lato_Medium_8);
  } else if (fontSize == 1) {
    display.setFont(&Lato_Medium_12);
  } else if (fontSize == 2) {
    display.setFont(&Lato_Medium_18);
  } else if (fontSize == 3) {
    display.setFont(&Lato_Medium_20);
  } else if (fontSize == 4) {
    display.setFont(&Lato_Medium_26);
  } else {
    Serial.println("ERROR: font size " + String(fontSize) + " is not supported, setting max size");
    display.setFont(&Lato_Medium_26);
  }
}

void displayBorder() {
  horizontalLine();
  verticalLine();
}

void horizontalLine() {
    // Line for showing end of display
    for (int16_t x = 0; x<displayWidth()+1; x++) {
      display.drawPixel(x,displayHeight()+1,0);
    }
}
void verticalLine() {
    // Line for showing end of display
    for (int16_t y = 0; y<displayHeight()+1; y++) {
      display.drawPixel(displayWidth()+1,y,0);
    }
}


// find the max length that fits the width
int fitMaxText(String text, int maxWidth) {
  int maxLength = 0;
  int16_t x1, y1;
  uint16_t w, h;

  // first get height of one big character
  display.getTextBounds("$", 0, 0, &x1, &y1, &w, &h);
  //Serial.println("Got big character bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(h) + " for text: $");
  uint16_t maxHeight = h * 1.5; // ensure it's really big, but smaller than 2 lines
  //Serial.println("maxHeight = " + String(maxHeight));
  h = 0;

  while (maxLength < text.length() && h < maxHeight && w < maxWidth) {
    String textToFit = text.substring(0, maxLength+2); // end is exclusive
    display.getTextBounds(textToFit, 0, 0, &x1, &y1, &w, &h);
    //Serial.println("Got text bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(h) + " for text: " + textToFit);
    maxLength++;
  }

  //Serial.println("Max text length that fits: " + String(maxLength));
  return maxLength;
}

// Try to fit a String into a rectangle.
// returns: the y position after fitting the text
int displayFit(String paymentDetail, int startX, int startY, int endX, int endY, int fontSize) {
  int yPos;

  while (fontSize > 0) {
    setFont(fontSize);

    // empty the entire rectangle
    display.fillRect(startX, startY, endX-startX, endY-startY, GxEPD_WHITE);

    yPos = startY;
    int textPos = 0;
    while (textPos < paymentDetail.length()) {
      // Try to fit everything that still needs displaying:
      String paymentDetailWithoutAlreadyPrintedPart = paymentDetail.substring(textPos);
      int chars = fitMaxText(paymentDetailWithoutAlreadyPrintedPart, endX);

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
    //Serial.println("After writing the paymentDetail, yPos = " + String(yPos) + " while endY = " + String(endY));

    // Check if the entire text fit:
    if (yPos < endY) {
      //Serial.println("yPos < endY so fontSize " + String(fontSize) + " fits!");
      break; // exit the fontSize loop because it fits
    } else {
      //Serial.println("fontSize " + String(fontSize) + " did not fit so trying smaller...");
      fontSize--;
    }
  }
  Serial.println("After fontSize loop, yPos = " + String(yPos));
  display.updateWindow(startX, startY, endX-startX, endY-startY, true);
  return yPos;
}

/*
 * returns: vertical cursor after printing balance
 */
int printBalance(int balance) {
    String walletBalanceText = String(balance) + " sats";

    int16_t x1, y1;
    uint16_t w, h;
    setFont(4);
    display.getTextBounds(walletBalanceText, 0, 0, &x1, &y1, &w, &h); // Got balance text bounds: 2,-19,181,20
    Serial.println("Got balance text bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(h)); // typical value for Lato_Medium_26: 1,-19,118,20
    display.setCursor(1, h);
    display.print(walletBalanceText);
    display.updateWindow(0,0,w+2,h+5,true); // for some mysterious reason, this needs a bit of extra margin around the text (2,5) instead of (1,0)
    return h;
}


void printTextCentered(char* str) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth() / 2 - w / 2,
                    displayHeight() / 2 - h / 2 );
    display.print(str);
}

void printTextCenteredX(String str, uint16_t yPos) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth() / 2 - w / 2, yPos );
    display.print(str);
}

void showLogo(const unsigned char logo [], int sizeX, int sizeY, int posX, int posY) {
  display.drawBitmap(logo, posX, posY, sizeX, sizeY, GxEPD_WHITE);
  display.updateWindow(posX, posY, sizeX, sizeY, true);
}
