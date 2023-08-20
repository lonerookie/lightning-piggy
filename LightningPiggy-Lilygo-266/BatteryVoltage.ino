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
    return voltage;
}

// Also displays LOW BATTERY warning
void displayVoltageAndLowBatteryWarning() {
    double voltage = getBatteryVoltage();

    if (voltage < 3.8) {
      setFont(3);
      display.setCursor(10,40);
      display.print((char*)String("LOW BATTERY!").c_str());
    }

    setFont(1);
    int16_t x1, y1;
    uint16_t w, h;
    int yPos = displayHeight();
    int xOffset = 1;

    // Sensor only works if wifi is connected
    String tempString = String(readTemp1(false), 1); // one digit after comma
    tempString += "C";
    const char *tempChar = tempString.c_str();
    display.getTextBounds((char*)tempChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w-xOffset,yPos);
    display.print((char*)tempChar);
    yPos = yPos - h - 1;

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
