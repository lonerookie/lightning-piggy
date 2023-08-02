double getBatteryVoltage() {
    // read and print battery level
    int totalLevel = 0;
    Serial.println("before reading...");
    for (int multiread=0; multiread<10; multiread++) {
      Serial.println("read " + String(multiread));
      totalLevel += analogRead(35);
      delay(100);
      Serial.println("after read " + String(multiread));
    }

    totalLevel = totalLevel / 10;
    Serial.println("Got battery level avg 10: " + String(totalLevel));

    double voltage = (totalLevel * 1.72) / 1000;  
}

void displayVoltage() {
    double voltage = getBatteryVoltage();

    //Serial.println("Calibrated voltage: " + String(voltage));

    //String batteryString(totalLevel);
    String voltageString(voltage);
    voltageString += "V";
    //const char *batteryChar = batteryString.c_str();
    const char *voltageChar = voltageString.c_str();
    display.setFont(&Lato_Medium_12);
    //display.setCursor(104,20);
    //display.print((char*)batteryChar);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds((char*)voltageChar, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(displayWidth()-w,104);
    display.print((char*)voltageChar);
}
