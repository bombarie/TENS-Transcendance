
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
  int id;

  if (m.remoteIP()  != oscRemoteIP) {
    oscRemoteIP = m.remoteIP();

    Serial.print("f:onTriggerReceived >> setting oscRemoteIP to ");
    Serial.println(oscRemoteIP);
  }

  if (m.size() == 5) {
    id = m.arg<int>(0);
    pulseWidth = m.arg<float>(1);
    pulseFrequency = m.arg<float>(2);
    pulseStrength = m.arg<float>(3);
    pulseDuration = m.arg<int>(4);

    if (bHasHeartbeat) {
      bool success = scheduleTrigger(
                       m.arg<int>(0), m.arg<float>(1), m.arg<float>(2), m.arg<float>(3), m.arg<int>(4)
                     );
      
      if (!success) {
        OscWiFi.send(oscRemoteIP, oscRemotePort, "/trigger", 0, m.arg<int>(0));
      }
    } else {
      Serial.println("System doesn't have active heartbeat -> not scheduling this trigger");
    }

    //    debugmsg("TRIGGER >> id: " + String(id)) ;

    Serial.print("trigger >> ");
    Serial.print(m.arg<int>(0)); // id
    Serial.print(", ");
    Serial.print(m.arg<float>(1)); // pulseWidth
    Serial.print(", ");
    Serial.print(m.arg<float>(2)); // pulseFrequency
    Serial.print(", ");
    Serial.print(m.arg<float>(3)); // pulseStrength
    Serial.print(", ");
    Serial.print(m.arg<int>(4)); // pulseDuration
    Serial.println();

  } else if (m.size() == 10) {
    id = m.arg<int>(0);
    pulseWidth = m.arg<float>(1);
    pulseFrequency = m.arg<float>(2);
    pulseStrength = m.arg<float>(3);
    pulseDuration = m.arg<int>(4);

    if (bHasHeartbeat) {
      bool success = scheduleDoubleTrigger(
                       m.arg<int>(0), m.arg<float>(1), m.arg<float>(2), m.arg<float>(3), m.arg<int>(4),
                       m.arg<int>(5), m.arg<float>(6), m.arg<float>(7), m.arg<float>(8), m.arg<int>(9)
                     );

      if (!success) {
        OscWiFi.send(oscRemoteIP, oscRemotePort, "/trigger", 0, m.arg<int>(0), m.arg<int>(5));
      }

    } else {
      Serial.println("System doesn't have active heartbeat -> not scheduling this trigger");
    }

    //    debugmsg("DOUBLE TRIGGER >> id 1: " + String(m.arg<int>(0)) + ", id 2: " + String(m.arg<int>(5)));

    Serial.print("double trigger >> ");
    Serial.print(m.arg<int>(0)); // id
    Serial.print(", ");
    Serial.print(m.arg<float>(1)); // pulseWidth
    Serial.print(", ");
    Serial.print(m.arg<float>(2)); // pulseFrequency
    Serial.print(", ");
    Serial.print(m.arg<float>(3)); // pulseStrength
    Serial.print(", ");
    Serial.print(m.arg<int>(4)); // pulseDuration
    Serial.print(" | ");
    Serial.print(m.arg<int>(5)); // id
    Serial.print(", ");
    Serial.print(m.arg<float>(6)); // pulseWidth
    Serial.print(", ");
    Serial.print(m.arg<float>(7)); // pulseFrequency
    Serial.print(", ");
    Serial.print(m.arg<float>(8)); // pulseStrength
    Serial.print(", ");
    Serial.print(m.arg<int>(9)); // pulseDuration
    Serial.println();

  } else {
    return;
  }



  //  Serial.print(m.remoteIP());
  //  Serial.print(" ");
  //  Serial.print(m.remotePort());
  //  Serial.print(" ");
  //  Serial.print(m.size());
  //  Serial.print(" ");
  //  Serial.print(m.address());
  //  Serial.print(" ");

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

  if (m.remoteIP()  != oscRemoteIP) {
    oscRemoteIP = m.remoteIP();

    Serial.print("f:onTriggerReceived >> setting oscRemoteIP to ");
    Serial.println(oscRemoteIP);
  }

  _prevHeartbeat = millis();
  if (!bHasHeartbeat) {
//    debugmsg("HEARTBEAT >> started receiving heartbeat");
    Serial.println("HEARTBEAT >> started receiving heartbeat");
    digitalWrite(LED_BUILTIN, HIGH);
    bHasHeartbeat = true;
  }

  // confirm heartbeat
  OscWiFi.send(oscRemoteIP, oscRemotePort, "/heartbeat", true);

}
