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

  Serial.println("Extracting time from received data");

  const char* datetime = doc["datetime"];
  String time = String(datetime);
  time = time.substring(time.indexOf("T") + 1, time.indexOf("T") + 6); // Extract only the time (hh:mm)

  Serial.println("Current time is: " + time);

  return time;
}