
void initWiFi() {
  // WiFi stuff (no timeout setting for WiFi)
#ifdef ESP_PLATFORM
  Serial.println("HELLO ESP_PLATFORM");
  WiFi.disconnect(true, true);  // disable wifi, erase ap info
  delay(1000);
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, pwd);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());

}

void onTriggerReceived(const OscMessage& m) {
  if (m.size() == 5) {
    
  }
  int id = m.arg<int>(0);
  pulseWidth = m.arg<float>(1);
  pulseFrequency = m.arg<float>(2);
  pulseStrength = m.arg<float>(3);
  pulseDuration = m.arg<int>(4);

  if (m.remoteIP()  != oscRemoteIP) {
    oscRemoteIP = m.remoteIP();
    
    Serial.print("f:onTriggerReceived >> setting oscRemoteIP to ");
    Serial.println(oscRemoteIP);
  }


  //  Serial.print(m.remoteIP());
  //  Serial.print(" ");
  //  Serial.print(m.remotePort());
  //  Serial.print(" ");
  //  Serial.print(m.size());
  //  Serial.print(" ");
  //  Serial.print(m.address());
  //  Serial.print(" ");

  Serial.print("trigger >> ");
  Serial.print(id);
  Serial.print(", ");
  Serial.print(pulseWidth);
  Serial.print(", ");
  Serial.print(pulseFrequency);
  Serial.print(", ");
  Serial.print(pulseStrength);
  Serial.print(", ");
  Serial.print(pulseDuration);
  Serial.println();

  debugmsg("received trigger event >> id: " + String(id) + ", pulseWidth: "  + String(pulseWidth) + ", pulseFrequency: "  + String(pulseFrequency) + ", pulseStrength: "  + String(pulseStrength) + ", pulseDuration: "  + String(pulseDuration)) ;

  if (bHasHeartbeat) {
    schedulePulse(id, pulseWidth, pulseFrequency, pulseStrength, pulseDuration);
  } else {
    Serial.println("System doesn't have active heartbeat -> not scheduling this trigger");
  }

  //  OscWiFi.send(host, send_port, "/reply", i, f, s);
}

void onHeartbeatReceived(const OscMessage& m) {
  //  Serial.print(m.remoteIP());
  //  Serial.print(" ");
  //  Serial.print(m.remotePort());
  //  Serial.print(" ");
  //  Serial.print(m.size());
  //  Serial.print(" ");
  //  Serial.print(m.address());
  //  Serial.println();
  //
  _prevHeartbeat = millis();
  if (!bHasHeartbeat) {
    debugmsg("HEARTBEAT >> stopped receiving heartbeat");
    Serial.println("HEARTBEAT >> started receiving heartbeat");
    digitalWrite(LED_BUILTIN, HIGH);
    bHasHeartbeat = true;
  }
}
