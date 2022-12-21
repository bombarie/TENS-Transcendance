
void onTriggerReceived(const OscMessage& m) {
  int id = m.arg<int>(0);
  pulseWidth = m.arg<float>(1);
  pulseFrequency = m.arg<float>(2);
  pulseStrength = m.arg<float>(3);
  pulseDuration = m.arg<int>(4);


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

  executePulse(id, pulseWidth, pulseFrequency, pulseStrength, pulseDuration);

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
    Serial.println("HEARTBEAT >> started receiving heartbeat");
    bHasHeartbeat = true;
  }
}
