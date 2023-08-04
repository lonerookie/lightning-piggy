
// Global variables for display
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);


void setup_display() {
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();

    // partial update to full screen to preset for partial update of box window (this avoids strange background effects)
    // this needs to be done before setRotation, otherwise still faint/missing pixels, even with using_rotation = true
    getDisplay().updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    getDisplay().setTextColor(GxEPD_BLACK);
    getDisplay().setRotation(1); // display is used in landscape mode
}

GxEPD_Class getDisplay() {
  return display;
}

void whiteDisplay() {
  getDisplay().fillScreen(GxEPD_WHITE); // erase entire display, otherwise old stuff might still be (faintly) there
  getDisplay().updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false); // seems needed to avoid artifacts later on when doing partial draws

  // temporarily display a border to simulate the new low resolution display
  displayBorder();
}

int displayHeight() {
  // lilygo 2.66: 152
  return 104; // lilygo 2.13
}

int displayWidth() {
  // lilygo 2.66: 296
  return 212; // lilygo 2.13
}

void displayBorder() {
  horizontalLine();
  verticalLine();
}

void horizontalLine() {
    // Line for showing end of display
    for (int16_t x = 0; x<displayWidth(); x++) {
      display.drawPixel(x,displayHeight()+1,0);
    }
}
void verticalLine() {
    // Line for showing end of display
    for (int16_t y = 0; y<displayHeight(); y++) {
      display.drawPixel(displayWidth()+1,y,0);
    }
}

void printBalance(int balance) {
    String walletBalanceText = String(balance) + " sats";

    int16_t x1, y1;
    uint16_t w, h;
    display.setFont(&Lato_Medium_26);
    display.getTextBounds(walletBalanceText, 0, 0, &x1, &y1, &w, &h);
    //Serial.println("Got text bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(walletBalanceTextHeight)); // typical value for Lato_Medium_26: 1,-19,118,20
    display.setCursor(1, h);
    display.print(walletBalanceText);
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
