#include "variables.h"

// #define ARDUINOOSC_DEBUGLOG_ENABLE

// Please include ArduinoOSCWiFi.h to use ArduinoOSC on the platform
// which can use both WiFi and Ethernet
#include <ArduinoOSCWiFi.h>
// this is also valid for other platforms which can use only WiFi
// #include <ArduinoOSC.h>

// send / receive varibales
int i;
float f;
String s;

long startPulseTime;
int _pulseDuration;
bool IsPulsing = false;


// SHOULD BE TRUE further down the line but I can see me
// disabling this during testing periods to prevent HAVING
// to have an active heartbeat.
bool bUseHeartbeat = false;


int currTriggerID;
float pulseWidth = 0.0;     // range [0.0, 1.0] -> in reality maps to [30µs, 260µs]
float pulseFrequency = 0.0; // range [0.0, 1.0] -> in reality maps to [2Hz, 150Hz]
float pulseStrength = 0.0;  // range [0.0, 1.0] -> in reality maps to [0V, 75V]
int pulseDuration = 0;      // milliseconds

void setup() {
  Serial.begin(115200);
  delay(2000);

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

  initDigiPot();

  pinMode(LED_BUILTIN, OUTPUT);

  initTENSIDToBoardPortMapping();
  initMuxBoards();

  OscWiFi.subscribe(recv_port, "/heartbeat", onHeartbeatReceived);
  OscWiFi.subscribe(recv_port, "/trigger", onTriggerReceived);

  startPulseTime = millis();
}

long prevMillisFast;
int updateIntervalFast = 1000 / 60;
long prevMillisSlow;
int updateIntervalSlow = 1000 / 30;

long mills;
int digipotApplyDuration = 190; // ms
bool digitpotIsSettling = false;

void loop() {
  mills = millis();

  // fast update loop
  if (mills - prevMillisFast > updateIntervalFast) {
    OscWiFi.update();  // should be called to receive + send osc

    prevMillisFast = mills;
  }


  // slow update loop
  if (mills - prevMillisSlow > updateIntervalSlow) {

    handleSerial();

    updateHeartbeat();

    if (bHasHeartbeat) {
      if (IsPulsing) {

        // cheap way to wait juuust a moment before applying the pulse
        if (digitpotIsSettling) {

          // if we have given the digipot time to settle, continue with the actual event
          if (millis() - startPulseTime > digipotApplyDuration) {
            startPulseTime += digipotApplyDuration;

            // *now* apply the trigger
            // setMux_1(IdToChannel[currTriggerID] - 1);
            setIdActive(currTriggerID);
            
            Serial.print(millis());
            Serial.println("start!");

            digitpotIsSettling = false;
          }
        }
        if (!digitpotIsSettling) {
          if (millis() - startPulseTime > _pulseDuration) {
            Serial.println("finished pulsing");
            IsPulsing = false;
          }
        }
      }
    }

    // turn off
    if (!bHasHeartbeat || !IsPulsing) {
      // set mux to 0
      // [to be implemented!] set output strength low, to be safe?

      pulseWidth = .1;
      pulseFrequency = .1;
      pulseStrength = 0.0;

      allMuxesOff();
    }

    //  POT0_Dn = (int)(pulseWidth * 255.0);
    //  POT1_Dn = (int)(pulseFrequency * 255.0);

    //    POT0_Dn++;
    //    POT0_Dn = POT0_Dn % 255;
    //  Serial.print(", POT0_Dn: ");
    //  Serial.println(POT0_Dn);

    //  Serial.print("pulseWidth: ");
    //  Serial.print(pulseWidth);

    updateDigiPot();


    prevMillisSlow = mills;
  }


  // FYI: how to send
  // OscWiFi.send(host, send_port, "/reply", i, f, s);

  // or do that separately
  // OscWiFi.parse(); // to receive osc
  // OscWiFi.post(); // to publish osc
}

void updateHeartbeat() {
  if (millis() - _prevHeartbeat > heartbeatTimeout) {
    if (bHasHeartbeat) {
      Serial.println("HEARTBEAT >> stopped receiving heartbeat");

      bHasHeartbeat = false;
    }
  }
}

void setIdActive(byte id) {
  allMuxesOff();

  if (id > 0 && id < 17) {
    byte board = id_to_boardport[id].board;
    byte port = id_to_boardport[id].port;
    switch (board) {
      case 0:
        setMux_1(port - 1);
        break;
      case 1:
        setMux_2(port - 1);
        break;
    }
  }

}

byte idCounter = 0;
bool handleSerial() {
  if (Serial.available()) {
    //    Serial.println("SERIAL");
    char c = Serial.read();
    switch (c) {
      case '1':
        Serial.print("idCounter++ : ");
        idCounter++;
        idCounter = idCounter % 17;
        setIdActive(idCounter);
        Serial.println(idCounter);
        return true;
        break;
      case '2':
        Serial.print("move pot 2 up >> value: ");
        if (POT1_Dn < 255) {
          POT1_Dn += 16;
        }
        Serial.println(POT1_Dn);
        return true;
        break;
      case '3':
        Serial.print("move pot 1 down >> value: ");
        if (POT0_Dn > 0) {
          POT0_Dn -= 16;
        }
        Serial.println(POT0_Dn);
        return true;
        break;
      case '4':
        Serial.print("move pot 2 down >> value: ");
        if (POT1_Dn > 0) {
          POT1_Dn -= 16;
        }
        Serial.println(POT1_Dn);
        return true;
        break;
    }

    while (Serial.available()) Serial.read();
  }
  return false;
}

void executePulse(int id, float pWidth, float pFreq, float pStrength, int duration) {
  // set right channel on 16-chan mux
  // set parameters of the digital pot
  // [to be implemented later!] set the strength
  // record the time now
  startPulseTime = millis();
  _pulseDuration = duration;
  IsPulsing = true;

  POT0_Dn = (int)(pWidth * 255.0);
  POT1_Dn = (int)(pFreq * 255.0);
  updateDigiPot();
  digitpotIsSettling = true;

  //  delay(500); // does this help the digipot update on time?

  id = constrain(id, 1, 16);
  currTriggerID = id;

  Serial.print(millis());
  Serial.println("pre-start...");
//  Serial.print("pre-start >> pulse id ");
//  Serial.print(currTriggerID);
//  Serial.print(", width ");
//  Serial.print(pWidth);
//  Serial.print(", frequency ");
//  Serial.print(pFreq);
//  Serial.print(", strength ");
//  Serial.print(pStrength);
//  Serial.print(", duration ");
//  Serial.println(duration);
}
