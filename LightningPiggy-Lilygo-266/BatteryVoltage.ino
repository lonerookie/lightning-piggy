extern float readTemp1(bool);

double getBatteryVoltage() {
    // read and print battery level
    int totalLevel = 0;
    // read multiple times to get an average value
    for (int multiread=0; multiread<3; multiread++) {
      totalLevel += analogRead(35);
      delay(100);
    }

    totalLevel = totalLevel / 3;
    Serial.println("Average battery level: " + String(totalLevel));

    double voltage = (totalLevel * 1.72) / 1000;
    //return 3.7; // for testing low battery situation
    return voltage;
}

// This function also displays the LOW BATTERY warning:
void displayHealthAndStatus() {
    double voltage = getBatteryVoltage();

    setFont(1);
    int16_t x1, y1;
    uint16_t w, h;
    int yPos = displayHeight();
    int xOffset = 1;

    /* Temperature sensor is missing and workaround shows too high or needs calibration...
    String tempString = String(readTemp1(false), 1); // one digit after comma
    tempString += "C";
    const char *tempChar = tempString.c_str();
    display.getTextBounds((char*)tempChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)tempChar);
    yPos = yPos - h - 1;
    */

    String voltageString(voltage, 2);
    voltageString += "V";
    const char *voltageChar = voltageString.c_str();
    display.getTextBounds((char*)voltageChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)voltageChar);
    yPos = yPos - h - 1;

    String hallString = String(read_internal_hall_sensor(), 2);
    hallString += "H";
    const char *hallChar = hallString.c_str();
    display.getTextBounds((char*)hallChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)hallChar);
    yPos = yPos - h - 1;

    String displayString = getShortDisplayInfo();
    const char *displayChar = displayString.c_str();
    display.getTextBounds((char*)displayChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)displayChar);
    yPos = yPos - h - 1;

    String versionString = "v" + getShortVersion();
    const char *versionChar = versionString.c_str();
    display.getTextBounds((char*)versionChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)versionChar);
    yPos = yPos - h - 1;

    float wifiStrength = getStrength(10);
    int wifiStrengthPercent = strengthPercent(wifiStrength);
    Serial.println("wifi strength: " + String(wifiStrength));
    Serial.println("wifi strength percent: " + String(wifiStrengthPercent));
    String wifiString = "WiFi:" + String(wifiStrengthPercent);
    const char *wifiChar = wifiString.c_str();
    display.getTextBounds((char*)wifiChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)wifiChar);
    yPos = yPos - h - 1;

    // Print big fat warning on top of everything if low battery
    if (voltage < 3.8) {
      setFont(2);
      const char * lowBatChar = " ! LOW BATTERY ! ";
      display.setCursor(1,displayHeight()-1);
      display.getTextBounds((char*)lowBatChar, 1, displayHeight()-1, &x1, &y1, &w, &h);
      Serial.println("Got lowBatChar bounds: " + String(x1) + "," + String(y1) + ","+ String(w) + "," + String(h));
      display.fillRect(x1, y1-4, w+4, h+4, GxEPD_BLACK);
      display.setTextColor(GxEPD_WHITE);
      display.print((char*)lowBatChar);
      display.setTextColor(GxEPD_BLACK);
    }
}

// returns true if battery was low
bool checkShowLowBattery() {
    // show warning if battery voltage is low
    double batteryVoltage = getBatteryVoltage();
    Serial.println("Battery voltage: " + String(batteryVoltage));
    if (batteryVoltage < 3.8) { // 3.7 is minimum but show warning a bit before
            String lowBatteryString = "Low battery (" + String(batteryVoltage) + " V)";
            const char *batteryChar = lowBatteryString.c_str();
            setFont(2);
            printTextCentered((char*)batteryChar);
            return true;
    } else {
      return false;
    }
}
