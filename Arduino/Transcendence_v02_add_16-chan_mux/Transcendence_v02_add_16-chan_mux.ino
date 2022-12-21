// #define ARDUINOOSC_DEBUGLOG_ENABLE

// Please include ArduinoOSCWiFi.h to use ArduinoOSC on the platform
// which can use both WiFi and Ethernet
#include <ArduinoOSCWiFi.h>
// this is also valid for other platforms which can use only WiFi
// #include <ArduinoOSC.h>

#include <SPI.h>

/***********************PIN Definitions*************************/
// set pin 10 as the slave select (SS) for the digital pot
// for using Arduino UNO
//const int CS_PIN = 10; // Arduino Nano
const int CS_PIN = 5; // Doit ESP32 Wroom
//const int CS_PIN = 15; // Wemos D1 Mini

/***********************MCP42XXX Commands************************/
//potentiometer select byte
const int POT0_SEL = 0x11;
const int POT1_SEL = 0x12;
const int BOTH_POT_SEL = 0x13;

//shutdown the device to put it into power-saving mode.
//In this mode, terminal A is open-circuited and the B and W terminals are shorted together.
//send new command and value to exit shutdowm mode.
const int POT0_SHUTDOWN = 0x21;
const int POT1_SHUTDOWN = 0x22;
const int BOTH_POT_SHUTDOWN = 0x23;

/***********************Customized Varialbes**********************/
//resistance value byte (0 - 255)
//The wiper is reset to the mid-scale position upon power-up, i.e. POT0_Dn = POT1_Dn = 128
int POT0_Dn = 128;
int POT1_Dn = 128;
int BOTH_POT_Dn = 128;

// WiFi @ Theater Utrecht
//const char* ssid = "pkat";
//const char* pwd = "th3@t3r1";

// WiFi @ home
const char* ssid = "WormKoning";
const char* pwd = "dingflofbips";

// @home
const IPAddress ip(192, 168, 1, 201);
const IPAddress gateway(192, 168, 1, 1);

// @theater utrecht
//const IPAddress ip(192, 168, 72, 201);
//const IPAddress gateway(192, 168, 72, 1);
const IPAddress subnet(255, 255, 255, 0);

// for ArduinoOSC
//const char* host = "192.168.72.185"; // remote host @theater utrecht
const char* host = "192.168.1.147"; // remote host @home
const int recv_port = 54321;
const int bind_port = 54345;
const int send_port = 55555;
const int publish_port = 54445;

// Mux control pins (esp32)
static byte s0 = 26;
static byte s1 = 25;
static byte s2 = 33;
static byte s3 = 32;

byte sigIndex = 0; // mux output channel

int IdToChannel[4] = { 0, 1, 2, 3 };

// send / receive varibales
int i;
float f;
String s;

long startPulseTime;
int _pulseDuration;
bool IsPulsing = false;

// SHOULD BE TRUE (further down the line)
bool bUseHeartbeat = false;



int currTriggerID;
float pulseWidth = 0.0;     // range [0.0, 1.0] -> in reality maps to [30µs, 260µs]
float pulseFrequency = 0.0; // range [0.0, 1.0] -> in reality maps to [2Hz, 150Hz]
float pulseStrength = 0.0;  // range [0.0, 1.0] -> in reality maps to [0V, 75V]
int pulseDuration = 0;      // milliseconds
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

void updateDigiPot() {
  DigitalPotWrite(POT0_SEL, POT0_Dn);  //set the resistance of POT0
  DigitalPotWrite(POT1_SEL, POT1_Dn);  //set the resistance of POT1
}

long _prevHeartbeat;
int heartbeatTimeout = 1000;
bool bHasHeartbeat = false;
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

void setup() {
  Serial.begin(115200);
  delay(2000);

  // WiFi stuff (no timeout setting for WiFi)
#ifdef ESP_PLATFORM
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


  // setup digital pot
  pinMode(CS_PIN, OUTPUT);  // set the CS_PIN as an output:
  SPI.begin();              // initialize SPI:

  DigitalPotWrite(BOTH_POT_SHUTDOWN, BOTH_POT_Dn);


  pinMode(LED_BUILTIN, OUTPUT);


  // init mux
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  setMux(15); // 15 is off / no-connected

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

            // now apply the trigger
            setMux(IdToChannel[currTriggerID] - 1);
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

      setMux(15); // chan 15 is off / no-connected
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

bool handleSerial() {
  if (Serial.available()) {
    //    Serial.println("SERIAL");
    char c = Serial.read();
    switch (c) {
      case '1':
        Serial.print("move pot 1 up >> value: ");
        if (POT0_Dn < 255) {
          POT0_Dn += 16;
        }
        Serial.println(POT0_Dn);
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

  Serial.println("pre-start...");
  //  Serial.print("starting pulse at id ");
  //  Serial.print(id);
  //  Serial.print(", width ");
  //  Serial.print(pWidth);
  //  Serial.print(", frequency ");
  //  Serial.print(pFreq);
  //  Serial.print(", strength ");
  //  Serial.print(pStrength);
  //  Serial.print(", duration ");
  //  Serial.println(duration);
}





void setMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  //  int val = digitalWrite(SIG_pin);

  //return the value
  //  return val;
}






//Function Declaration
void DigitalPotTransfer(int cmd, int value);  //send the command and the resistance value through SPI

void DigitalPotWrite(int cmd, int val) {
  //  SPI.beginTransaction(SPISettings(SCK, MSBFIRST, SPI_MODE0));
  // constrain input value within 0 - 255
  val = constrain(val, 0, 255);
  // set the CS pin to low to select the chip:
  digitalWrite(CS_PIN, LOW);
  // send the command and value via SPI:
  SPI.transfer(cmd);
  SPI.transfer(val);
  // Set the CS pin high to execute the command:
  digitalWrite(CS_PIN, HIGH);
  //  SPI.endTransaction();
}
