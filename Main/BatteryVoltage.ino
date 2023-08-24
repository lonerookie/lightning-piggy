
// Returns the VBAT value.
// If we're not on battery but USB powered, then it returns a negative VBAT value.
double getBatteryVoltage() {
    const int nrReads = 4;

    int totalDiff = 0;

    int prevValue = analogRead(35); delay(10);
    int totalLevel = prevValue;

    for (int multiread=0; multiread<nrReads; multiread++) {
      int value = analogRead(35);
      totalLevel += value;
      totalDiff += abs(value - prevValue);
      delay(100);
    }

    totalLevel = totalLevel / (nrReads+1); // one additional read is done for the prevValue
    totalDiff = totalDiff / nrReads;
    Serial.println("Average battery level: " + String(totalLevel));
    Serial.println("Average battery level diff: " + String(totalDiff));

    double voltage = (totalLevel * 1.72) / 1000;
    if (totalDiff > 32) {
      // The battery has a smoothing effect so lots of variance between levels means we're not on battery.
      voltage *= -1;
    }
    //return 3.7; // for testing low battery situation
    return voltage;
}

// This function also displays the LOW BATTERY warning:
void displayHealthAndStatus() {
    setFont(1);
    int16_t x1, y1;
    uint16_t w, h;
    int verticalSpace = 2; // space between each item
    int yPos = displayHeight() - verticalSpace;
    int xOffset = 1;
    int minX = displayWidth(); // track min X to know which area of display to update

    /* Temperature sensor is missing and workaround shows too high or needs calibration...
    String tempString = String(readTemp1(false), 1); // one digit after comma
    tempString += "C";
    const char *tempChar = tempString.c_str();
    display.getTextBounds((char*)tempChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)tempChar);
    yPos = yPos - h - 1;
    */

    double voltage = getBatteryVoltage();
    String voltageString = "NOBAT";
    if (voltage > 0) {
      voltageString = String(voltage, 2) + "V";
    }
    const char *voltageChar = voltageString.c_str();
    display.getTextBounds((char*)voltageChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    minX = min(displayWidth()-w-xOffset,minX);
    display.print((char*)voltageChar);
    yPos = yPos - h - verticalSpace;

    String hallString = String(read_internal_hall_sensor(), 2);
    hallString += "H";
    const char *hallChar = hallString.c_str();
    display.getTextBounds((char*)hallChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    minX = min(displayWidth()-w-xOffset,minX);
    display.print((char*)hallChar);
    yPos = yPos - h - verticalSpace;

    String displayString = getShortDisplayInfo();
    const char *displayChar = displayString.c_str();
    display.getTextBounds((char*)displayChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    minX = min(displayWidth()-w-xOffset,minX);
    display.print((char*)displayChar);
    yPos = yPos - h - verticalSpace;

    String versionString = "v" + getShortVersion();
    const char *versionChar = versionString.c_str();
    display.getTextBounds((char*)versionChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    minX = min(displayWidth()-w-xOffset,minX);
    display.print((char*)versionChar);
    yPos = yPos - h - verticalSpace;

    String wifiString = "Wifi:";
    if (wifiConnected()) {
      int wifiStrengthPercent = strengthPercent(getStrength(5));
      Serial.println("wifi strength percent: " + String(wifiStrengthPercent));
      wifiString += String(wifiStrengthPercent) + "%";
    } else {
      wifiString += "off";
    }
    const char *wifiChar = wifiString.c_str();
    display.getTextBounds((char*)wifiChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    minX = min(displayWidth()-w-xOffset,minX);
    display.print((char*)wifiChar);
    yPos = yPos - h - verticalSpace;

    //Serial.println("minX,yPos = " + String(minX) + "," + String(yPos)); // minX,yPos = 192,67
    updateWindow(minX, yPos, displayWidth()-minX, displayHeight()-yPos);
}

// returns true if voltage is low, false otherwise
bool displayVoltageWarning() {
    int16_t x1, y1;
    uint16_t w, h;
    double voltage = getBatteryVoltage();
    // Print big fat warning on top of everything if low battery
    if (voltage > 0 && voltage < 3.8) {
      setFont(2);
      String lowBatString = " ! LOW BATTERY (" + String(voltage) + "V) ! ";
      const char * lowBatChar = lowBatString.c_str();
      display.setCursor(1,displayHeight()-1);
      display.getTextBounds((char*)lowBatChar, 1, displayHeight()-1, &x1, &y1, &w, &h);
      Serial.println("Got lowBatChar bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(h));
      display.fillRect(x1, y1-4, w+4, h+4, GxEPD_BLACK);
      display.setTextColor(GxEPD_WHITE);
      display.print((char*)lowBatChar);
      updateWindow(x1, y1-4, w+4, h+4);
      display.setTextColor(GxEPD_BLACK);
      return true;
    } else {
      return false;
    }
}
