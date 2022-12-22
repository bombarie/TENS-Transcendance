#include "variables.h"



/*******   WIFI settings   ******************************/

// SSID and password of the wifi access point
const char* ssid = "ORBI06";
const char* pwd = "livelyprairie064";
 
// IP address for the Arduino
const IPAddress ip(10, 0, 0, 88);
const IPAddress gateway(10, 0, 0, 1);

/*******************************************************/




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
int currTriggerIDs[2];
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

            allMuxesOff();
            for (int i = 0; i < 2; i++) {
              if (currTriggerIDs[i] != -1) {
                setIdActive(currTriggerIDs[i]);
              }
            }
            //            setIdActive(currTriggerID);

            //            Serial.print(millis());
            //            Serial.println(" start!");

            digitpotIsSettling = false;
          }
        }
        if (!digitpotIsSettling) {
          if (millis() - startPulseTime > _pulseDuration) {
            //            Serial.print(millis());
            //            Serial.println(" finished pulsing");
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
        executeTriggersAtIndex(triggerListIndex);
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
  //  Serial.println("f:debugmsg");

  if (oscRemotePort != 0) {
    OscWiFi.send(oscRemoteIP, oscRemotePort, "/debugmsg", s);
  } else {
    Serial.println("f:debugmsg ERROR >> oscRemotePort is not initialized >> not sending");
  }
}

void updateHeartbeat() {
  if (millis() - _prevHeartbeat > heartbeatTimeout) {
    if (bHasHeartbeat) {
      //      debugmsg("HEARTBEAT >> stopped receiving heartbeat");
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
  Serial.println("f:setIdActive >> id: " + String(id));

  if (id > 0 && id < 22) {
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
        // spawn one entry with random values
        if (random(0, 1000) < 500) {
          // schedule one trigger
          scheduleTrigger((int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(200, 2000));
        } else {
          // schedule two triggers

          int dur = random(200, 2000);
          scheduleDoubleTrigger(
            (int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, dur,
            (int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, dur
          );
        }
        return true;
        break;
      case '3':
        Serial.println("Scheduling 5 pulses");
        // spawn 5 entries with random values
        for (int i = 0 ; i < 5; i++) {
          if (random(0, 1000) < 500) {
            // schedule one trigger
            scheduleTrigger((int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(200, 2000));
          } else {
            // schedule two triggers

            int dur = random(200, 2000);
            scheduleDoubleTrigger(
              (int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, dur,
              (int)random(1, 17), random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, random(0, 1000) / 1000.0, dur
            );
          }
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



void executeTriggersAtIndex(byte i) {
  Serial.print("f:executeTriggersAtIndex >> activating ");
  for (int j = 0; j < tensTriggerList[i].numTriggers; j++) {
    currTriggerIDs[j] = -1;
    if (tensTriggerList[i].tensTriggers[j].isActive) {
      currTriggerIDs[j] = tensTriggerList[i].tensTriggers[j].id;
      if (j > 0) {
        Serial.print(", ");
      } else {

      }
      Serial.print("id[");
      Serial.print(j);
      Serial.print("] >> id: ");
      Serial.print(tensTriggerList[i].tensTriggers[j].id);

      executePulse(tensTriggerList[i].tensTriggers[j].pulseWidth, tensTriggerList[i].tensTriggers[j].pulseFrequency, tensTriggerList[i].tensTriggers[j].pulseStrength, tensTriggerList[i].tensTriggers[j].duration);
    }
  }


  Serial.println();

  // send feedback -> first option is true for a single trigger (ie the second array index is not set)
  if (currTriggerIDs[1] == -1) {
    OscWiFi.send(oscRemoteIP, oscRemotePort, "/trigger", 1, currTriggerIDs[0]);
  } else {
    OscWiFi.send(oscRemoteIP, oscRemotePort, "/trigger", 1, currTriggerIDs[0], currTriggerIDs[1]);
  }

  Serial.print("f:executeTriggersAtIndex >> currTriggerIDs[0]: ");
  Serial.print(currTriggerIDs[0]);
  Serial.print(", currTriggerIDs[1]: ");
  Serial.println(currTriggerIDs[1]);


}
//void executePulse(int id, float pWidth, float pFreq, float pStrength, int duration) {
void executePulse(float pWidth, float pFreq, float pStrength, int duration) {
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

  //  id = constrain(id, 1, 21);
  //  currTriggerID = id;

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

bool scheduleTrigger(int id, float pWidth, float pFreq, float pStrength, int duration) {
  id = constrain(id, 1, 21);

  if (!id_to_boardport[id].isSet) {
    Serial.println("scheduleTrigger >> id ");
    Serial.print(id);
    Serial.println(" is not assigned to a board >> rejecting command");

    return false;
  }

  triggerListActiveHeadIndex++;
  triggerListActiveHeadIndex = triggerListActiveHeadIndex % triggerListLength;

  tensTriggerList[triggerListActiveHeadIndex] = TENSTriggerHolder(
        TENSTrigger(id, pWidth, pFreq, pStrength, duration)
      );

  Serial.print("scheduleTrigger >> id ");
  Serial.print(id);
  Serial.print(", width ");
  Serial.print(pWidth);
  Serial.print(", frequency ");
  Serial.print(pFreq);
  Serial.print(", strength ");
  Serial.print(pStrength);
  Serial.print(", duration ");
  Serial.println(duration);

  return true;
}

bool scheduleDoubleTrigger(
  int id1, float pWidth1, float pFreq1, float pStrength1, int duration1,
  int id2, float pWidth2, float pFreq2, float pStrength2, int duration2
) {
  id1 = constrain(id1, 1, 21);
  id2 = constrain(id2, 1, 21);

  // TODO: check if both id's are not in the same board
  if (!id_to_boardport[id1].isSet || !id_to_boardport[id2].isSet) {
    // one or both of the ids are not set -> for now: reject the command
    Serial.println("scheduleDoubleTrigger >> one or both id's not assigned to a board >> rejecting command");
    return false;
  }

  if (id_to_boardport[id1].board == id_to_boardport[id2].board) {
    // id's are on the same board -> for now: reject the command
    Serial.println("scheduleDoubleTrigger >> both id's are assigned to the same board >> rejecting command");
    return false;
  }

  triggerListActiveHeadIndex++;
  triggerListActiveHeadIndex = triggerListActiveHeadIndex % triggerListLength;

  tensTriggerList[triggerListActiveHeadIndex] = TENSTriggerHolder(
        TENSTrigger(id1, pWidth1, pFreq1, pStrength1, duration1),
        TENSTrigger(id2, pWidth2, pFreq2, pStrength2, duration2)
      );

  Serial.print("scheduleDoubleTrigger >> id_1 ");
  Serial.print(id1);
  Serial.print(", width_1 ");
  Serial.print(pWidth1);
  Serial.print(", frequency_1 ");
  Serial.print(pFreq1);
  Serial.print(", strength_1 ");
  Serial.print(pStrength1);
  Serial.print(", duration_1 ");
  Serial.print(duration1);
  Serial.print(", id_2 ");
  Serial.print(id2);
  Serial.print(", width_2 ");
  Serial.print(pWidth2);
  Serial.print(", frequency_2 ");
  Serial.print(pFreq2);
  Serial.print(", strength_2 ");
  Serial.print(pStrength2);
  Serial.print(", duration_2 ");
  Serial.println(duration2);

  return true;

}
