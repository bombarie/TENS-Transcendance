#ifndef TENS_VARIABLES_H
#define TENS_VARIABLES_H


/***********************
   WIFI
*/


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




/***********************
   OSC
*/

// for ArduinoOSC
//const char* host = "192.168.72.185"; // remote host @theater utrecht
const char* host = "192.168.1.147"; // remote host @home
const int recv_port = 54321;
const int bind_port = 54345;
const int send_port = 55555;
const int publish_port = 54445;

long _prevHeartbeat;
int heartbeatTimeout = 1000;

// SHOULD BE TRUE (further down the line).
// If is false, no TENS output should be enabled (or in very least 
// put pulse width/freq to lowest setting).
bool bHasHeartbeat = false;





/***********************
   DIGIPOT
*/

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




/***********************
   MUX boards
*/

#define MUX_OFF_PIN 15
// Mux control pins (esp32)

// TODO refactor to the grander scheme
int IdToChannel[4] = { 3, 4, 5, 6 };

struct IdToBoardPort {

  IdToBoardPort(): board(255), port(255) {}
  IdToBoardPort(byte _board, byte _port): board(_board), port(_port) {}
  
  byte board;
  byte port;
};

IdToBoardPort id_to_boardport[22];


// set #1
static byte mux_1_s0 = 13; // 13
static byte mux_1_s1 = 12; // 12
static byte mux_1_s2 = 14; // 14
static byte mux_1_s3 = 27; // 27

// set #2
static byte mux_2_s0 = 26;
static byte mux_2_s1 = 25;
static byte mux_2_s2 = 33;
static byte mux_2_s3 = 32;

// set #3
static byte mux_3_s0 = 15; // GUESSTIMATE -> TODO: CONFIRM
static byte mux_3_s1 = 2; // GUESSTIMATE -> TODO: CONFIRM
static byte mux_3_s2 = 4; // GUESSTIMATE -> TODO: CONFIRM
static byte mux_3_s3 = 19; // GUESSTIMATE -> TODO: CONFIRM






/***********************
   SOMETHING
*/









#endif
