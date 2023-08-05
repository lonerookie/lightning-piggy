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
      display.setFont(&Lato_Medium_20);
      display.setCursor(10,40);
      display.print((char*)String("LOW BATTERY!").c_str());
    }

    String voltageString(voltage);
    voltageString += "V";
    const char *voltageChar = voltageString.c_str();
    display.setFont(&Lato_Medium_12);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds((char*)voltageChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w,104);
    display.print((char*)voltageChar);
}

// returns true if battery was low
bool checkShowLowBattery() {
    // show warning if battery voltage is low
    double batteryVoltage = getBatteryVoltage();
    Serial.println("Battery voltage: " + String(batteryVoltage));
    if (batteryVoltage < 3.8) { // 3.7 is minimum but show warning a bit before
            String lowBatteryString = "Low battery (" + String(batteryVoltage) + " V)";
            const char *batteryChar = lowBatteryString.c_str();
            display.setFont(&Lato_Medium_18);
            printTextCentered((char*)batteryChar);
            return true;
    } else {
      return false;
    }
}
