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

int digipotApplyDuration = 190; // ms
bool digitpotIsSettling = false;

void setup() {
  Serial.begin(115200);
  delay(2000);

  initWiFi();

  initDigiPot();

  pinMode(LED_BUILTIN, OUTPUT);

  initTENSIDToBoardPortMapping();
  initMuxBoards();

  OscWiFi.subscribe(recv_port, "/heartbeat", onHeartbeatReceived);
  OscWiFi.subscribe(recv_port, "/trigger", onTriggerReceived);

  startPulseTime = millis();
}

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


    // TODO merge the logic of the 'IsPulsing' conditional below and the updateTENSTriggerList()
    updateTENSTriggerList();

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

//            Serial.print(millis());
//            Serial.println("start!");

            digitpotIsSettling = false;
          }
        }
        if (!digitpotIsSettling) {
          if (millis() - startPulseTime > _pulseDuration) {
            Serial.println("finished pulsing");
            tensTriggerList[triggerListIndex].isActive = false;
            IsPulsing = false;
          }
        }
      }
    }

    // turn off
    if (!bHasHeartbeat || !IsPulsing) {
      // set mux to 0
      // [to be implemented!] *actually* set output strength low, to be safe? -> like through a relay?

      pulseWidth = .1;
      pulseFrequency = .1;
      pulseStrength = 0.0;

      allMuxesOff();
    }

    updateDigiPot();

    prevMillisSlow = mills;
  }

  // FYI: how to send
  // OscWiFi.send(host, send_port, "/reply", i, f, s);

  // or do that separately
  // OscWiFi.parse(); // to receive osc
  // OscWiFi.post(); // to publish osc
}


void updateTENSTriggerList() {
  // commented out the wait-until-make-inactive conditional -> it's handled in the main loop()
  // TODO: MERGE!

  //  if (tensTriggerList[triggerListIndex].isActive) {
  //    // wait to deactivate
  //    if (millis() - triggerTimestamp > tensTriggerList[triggerListIndex].duration) {
  //      // deactivate
  //
  //      Serial.print("Done with trigger at index ");
  //      Serial.print(triggerListIndex);
  //      Serial.print(" after ");
  //      Serial.print(millis() - triggerTimestamp);
  //      Serial.println(" ms");
  //
  //      tensTriggerList[triggerListIndex].isActive = false;
  //    }
  //  }
  if (!tensTriggerList[triggerListIndex].isActive) {
    if (triggerListActiveHeadIndex != triggerListIndex) {
      triggerListIndex++;
      triggerListIndex = triggerListIndex % triggerListLength;

      // This if statement shouldn't be necessary because a 'new index'
      // should de facto mean that a new trigger was added to the list,
      // but hey... let's double-check. In the worst case this prevents
      // some runaway event.
      if (tensTriggerList[triggerListIndex].isActive) {
        // activate
        executePulseAtIndex(triggerListIndex);
      }
    }
  }
}

// set the digipot and such
void setTriggerActive(byte i) {
  triggerTimestamp = millis();
  tensTriggerList[i].isActive = true;
}


// send a debug string over osc 
void debugmsg(String s) {
  Serial.println("f:debugmsg");
  if (oscRemotePort != 0) {
    OscWiFi.send(oscRemoteIP, oscRemotePort, "/debugmsg", s);
//    OscWiFi.send(oscRemoteIP, oscRemotePort, "/debugmsg", 11, 12, 13);
  } else {
  Serial.println("  oscRemotePort is not initialized >> not sending");
  }
}

void updateHeartbeat() {
  if (millis() - _prevHeartbeat > heartbeatTimeout) {
    if (bHasHeartbeat) {
      debugmsg("HEARTBEAT >> stopped receiving heartbeat");
      Serial.println("HEARTBEAT >> stopped receiving heartbeat");

      // just to be safe -> always clear any buffered TENS triggers?
      triggerListIndex = triggerListActiveHeadIndex;
      tensTriggerList[triggerListIndex].isActive = false;

      digitalWrite(LED_BUILTIN, LOW);

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
        Serial.println("Scheduling 1 pulse");
        // spawn one entry with random values
        schedulePulse((int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(200, 2000));
        return true;
        break;
      case '3':
        Serial.println("Scheduling 5 pulses");
        // spawn 5 entries with random values
        for (int i = 0 ; i < 5; i++) {
          schedulePulse((int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(200, 2000));
        }
        return true;
        break;
      case '4':
        return true;
        break;
    }

    while (Serial.available()) Serial.read();
  }
  return false;
}



void executePulseAtIndex(byte i) {
  executePulse(tensTriggerList[i].id, tensTriggerList[i].pulseWidth, tensTriggerList[i].pulseFrequency, tensTriggerList[i].pulseStrength, tensTriggerList[i].duration);
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

  id = constrain(id, 1, 16);
  currTriggerID = id;

//  Serial.print(millis());
//  Serial.println(" >> pre-start...");
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

void schedulePulse(int id, float pWidth, float pFreq, float pStrength, int duration) {
  id = constrain(id, 1, 16);

  triggerListActiveHeadIndex++;
  triggerListActiveHeadIndex = triggerListActiveHeadIndex % triggerListLength;
  tensTriggerList[triggerListActiveHeadIndex] = TENSTrigger(id, pWidth, pFreq, pStrength, duration);

  Serial.print("schedulePulse >> id ");
  Serial.print(id);
  Serial.print(", width ");
  Serial.print(pWidth);
  Serial.print(", frequency ");
  Serial.print(pFreq);
  Serial.print(", strength ");
  Serial.print(pStrength);
  Serial.print(", duration ");
  Serial.println(duration);

}
