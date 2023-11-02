String getTimeFromNTP() {
  String timeData = getEndpointData("worldtimeapi.org", "/api/timezone/" + String(timezone), false);

  DynamicJsonDocument doc(8192); 

  DeserializationError error = deserializeJson(doc, timeData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return "na";
  }

  Serial.println("Extracting weekday and time from received data");

  String datetimeAsString = String(doc["datetime"]);

  int dayOfWeek = doc["day_of_week"];
  String dayOfWeekAsString = getDayOfWeekString(dayOfWeek);
  String time = datetimeAsString.substring(datetimeAsString.indexOf("T") + 1, datetimeAsString.indexOf("T") + 6); // Extract only the time (hh:mm)

  return dayOfWeekAsString + " " + time;
}

String getDayOfWeekString(int dayOfWeek) {
  if (settingLanguage == LANGUAGE_EN_US) {
     switch(dayOfWeek) {
        case 0:
          return "Mon";
        case 1:
          return "Tue";
        case 2:
          return "Wed";
        case 3:
          return "Thu";
        case 4:
          return "Fri";
        case 5:
          return "Sat";
        case 6:
          return "Sun";
    }
  }
  else if (settingLanguage == LANGUAGE_DA) {
     switch(dayOfWeek) {
        case 0:
          return "Man";
        case 1:
          return "Tir";
        case 2:
          return "Ons";
        case 3:
          return "Tors";
        case 4:
          return "Fre";
        case 5:
          return "Lør";
        case 6:
          return "Søn";
    }
  }

  return "NA";
}