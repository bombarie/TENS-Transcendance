/*

 *** OSC protocol
 Packet of 5 arguments: ifffi
 int: ID of elektrode pair to trigger (1-21) (21 elektrode pairs, no?)
 float: pulse width (0.0-1.0)
 float: pulse frequency (0.0-1.0)
 float: pulse strength (0.0-1.0)
 int: pulse duration
 
 
 ***
 v02
 -> send commands for four elektrodes
 
 
 ***
 v03
 -> receive udp from touchdesigner (for some reason wasn't possible to send osc packets)
 and convert this to an osc packet to the ESP32
 
 ***
 v06
 
 The current separation of ID's over the boards:
 - board 1: 1, 2, 3, 4, 5, 16, 17, 18, 19, 20
 - board 2: 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 21
 
 
 */

import hypermedia.net.*;
import oscP5.*;
import netP5.*;
import controlP5.*;

UDP udp;  // define the UDP object

OscP5 oscP5;
NetAddress oscRemoteESP32;
NetAddress oscRemoteTD;

boolean initOSCToTDInitted = false;

ControlP5 cp5;

PFont font_andale;
PFont font_andale_12;
int udpReceivePort = 7001;


String oscTDTargetIP = "";
int oscTDTargetPort = 7002;

String oscESP32TargetIP = "10.0.0.88";
int oscReceivePort = 7075;
int oscESP32TargetPort = 54321;

boolean isReceivingHeartbeat = false;
int lastHeartBeat_timestamp;
int heartbeatTimeout = 1500; // ms

Button btn_singletrigger;
Button btn_doubletrigger_valid;
Button btn_doubletrigger_invalid;

Textarea connectionTextArea;


void setup() {
  size(1280, 480);

  font_andale = loadFont("AndaleMono-16.vlw");
  font_andale_12 = loadFont("AndaleMono-12.vlw");
  textFont(font_andale, 13);
  textAlign(LEFT, TOP);

  initOSCToESP32();
  initUDP();

  initControlP5();

  frameRate(30);

  printLog("started");
}

void draw() {
  background(#3b6662);
  fill(255);
  //text("keys z,a,q,1 to control elektrode pair #1 in increasing intensities", 15, 20);
  //text("keys x,s,w,2 to control elektrode pair #2 in increasing intensities", 15, 40);
  //text("keys c,d,e,3 to control elektrode pair #3 in increasing intensities", 15, 60);
  //text("keys v,f,r,4 to control elektrode pair #4 in increasing intensities", 15, 80);

  if (millis() - lastHeartBeat_timestamp > heartbeatTimeout) {
    if (isReceivingHeartbeat) {
      printLog("HEARTBEAT - connection lost");
      isReceivingHeartbeat = false;
    }
  }

  drawConnectionSettings(15, 20);
  drawHeartbeatConnection(15, 320);
  drawDebugButtons(15, 390);

  drawConnectionLog(250, 20);
  updateConnectionLog();

  if (frameCount % 10 == 0) {
    sendHeartbeat();
  }
}

void initOSCToESP32() {
  oscP5 = new OscP5(this, oscReceivePort);

  // @home : 192.168.1.201
  // @theater : 192.168.72.201
  // @theater - production AP : 10.0.0.88
  oscRemoteESP32 = new NetAddress(oscESP32TargetIP, oscESP32TargetPort);
}

void initOSCToTD() {
  oscRemoteTD = new NetAddress(oscTDTargetIP, oscTDTargetPort);
  initOSCToTDInitted = true;
}

void initUDP() {
  udp = new UDP( this, udpReceivePort );

  //udp.log( true );     // <-- printout the connection activity
  udp.listen( true );
}

void initControlP5() {
  cp5 = new ControlP5(this);

  connectionTextArea = cp5.addTextarea("txt")
    .setPosition(250, 45)
    .setSize(1010, 415)
    //.setFont(createFont("arial",12))
    .setFont(font_andale_12)
    .setLineHeight(14)
    .setColor(color(200))
    .setColorBackground(color(0, 200))
    .setColorForeground(color(255, 255));
  ;

  // TEST TEXT
  /*
  connectionTextArea.setText("Lorem Ipsum is simply dummy text of the printing and typesetting"
   +" industry. Lorem Ipsum has been the industry's standard dummy text"
   +" ever since the 1500s, when an unknown printer took a galley of type"
   +" and scrambled it to make a type specimen book. It has survived not"
   +" only five centuries, but also the leap into electronic typesetting,"
   +" remaining essentially unchanged. It was popularised in the 1960s"
   +" with the release of Letraset sheets containing Lorem Ipsum passages,"
   +" and more recently with desktop publishing software like Aldus"
   +" PageMaker including versions of Lorem Ipsum."
   );
   //*/

  btn_singletrigger = cp5.addButton("single trigger")
    .setValue(0)
    .setPosition(100, 100)
    .setSize(150, 19)
    ;

  btn_doubletrigger_valid = cp5.addButton("double trigger (valid)")
    .setValue(0)
    .setPosition(100, 125)
    .setSize(150, 19)
    ;

  btn_doubletrigger_invalid = cp5.addButton("double trigger (invalid)")
    .setValue(0)
    .setPosition(100, 150)
    .setSize(150, 19)
    ;
}


void drawConnectionSettings(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("CONNECTION SETTINGS", 0, 0);

  text("*** OSC ( P5->ESP32 ) ***", 0, 40);
  text("target ip: " + oscESP32TargetIP, 0, 60);
  text("target port: " + oscESP32TargetPort, 0, 80);
  text("receive port: " + oscReceivePort, 0, 100);

  text("*** OSC ( ESP32->P5 )  ***", 0, 140);
  text("receive port: " + udpReceivePort, 0, 160);

  text("*** OSC ( P5->TD )  ***", 0, 200);
  if (initOSCToTDInitted) {
    text("target ip: " + oscTDTargetIP, 0, 220);
    text("target port: " + oscTDTargetPort, 0, 240);
  } else {
    text("not initted (have not \nreceived a udp packet yet)", 0, 220);
  }

  popMatrix();
}

void drawHeartbeatConnection(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("HEARTBEAT FROM TENS ESP32", 0, 0);

  ellipseMode(CORNER);
  noStroke();
  int ellipseRadius = 10;
  if (isReceivingHeartbeat) {
    fill(0, 240, 0);
    ellipse(0, 20, ellipseRadius, ellipseRadius);
    text("connected", ellipseRadius + 10, 10 + ellipseRadius);
  } else {
    fill(240, 0, 0);
    ellipse(0, 20, ellipseRadius, ellipseRadius);
    text("not connected", ellipseRadius + 10, 10 + ellipseRadius);
  }


  popMatrix();
}

void drawDebugButtons(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("TEST TRIGGERS", 0, 0);
  btn_singletrigger.setPosition(x + 0, y + 20);
  btn_doubletrigger_valid.setPosition(x + 0, y + 40);
  btn_doubletrigger_invalid.setPosition(x + 0, y + 60);

  popMatrix();
}

void drawConnectionLog(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("COMMUNICATION LOG", 0, 0);

  popMatrix();
}

void updateConnectionLog() {
  int lines = split(connectionTextArea.getText(), "\n").length;

  if (lines > 50) {
    // find index of last carriage return
    int lastCRLoc = connectionTextArea.getText().lastIndexOf("\n");

    // shorten the textarea to that last index (ie: remove the last line)
    connectionTextArea.setText(connectionTextArea.getText().substring(0, lastCRLoc));
  }
}

public void controlEvent(ControlEvent theEvent) {
  //println("controlEvent: " + theEvent.getController().getName());

  switch(theEvent.getController().getName()) {
  case "single trigger":
    //println("     found 'single'");
    int randID = (int)random(1, 6);
    sendTENSTrigger(randID, .6f, .55f, .3f, 850);
    break;
  case "double trigger (valid)":
    //println("     found 'double ok'");
    sendTENSDoubleTrigger(
      3, .66f, .6f, .3f, 850,
      13, .66f, .6f, .3f, 850
      );
    break;
  case "double trigger (invalid)":
    //println("     found 'double not ok'");
    sendTENSDoubleTrigger(
      4, .66f, .6f, .3f, 850,
      17, .66f, .6f, .3f, 850
      );
    break;
  }
}

void tryParseAndSendThroughOSC(String message) {
  message = message.trim();

  String[] values = split(message, "  ");
  //println("f:tryParseAndSendThroughOSC >> picked apart example packet into " + values.length + " parts");

  if (values.length != 5 && values.length != 10) {
    println("f:tryParseAndSendThroughOSC >> message did not contain 5 or 10 (double-trigger) arguments >> skipping");
    printLog("TD >> UDP >> ERROR >> received a bogus packet (did not contain 5 or 10 arguments)");
    return;
  }

  if (values.length == 5) {
    sendTENSTrigger(
      parseInt(values[0]), parseFloat(values[1]), parseFloat(values[2]), parseFloat(values[3]), parseInt(values[4])
      );

    printLog("TD >> UDP >> received single ID trigger >> id: " + parseInt(values[0]));
  }
  if (values.length == 10) {
    sendTENSDoubleTrigger(
      parseInt(values[0]), parseFloat(values[1]), parseFloat(values[2]), parseFloat(values[3]), parseInt(values[4]),
      parseInt(values[5]), parseFloat(values[6]), parseFloat(values[7]), parseFloat(values[8]), parseInt(values[9])
      );

    printLog("TD >> UDP >> received double ID trigger >> id 1: " + parseInt(values[0]) + ", id 2: " + parseInt(values[5]));
  }

  //int id = parseInt(values[0]);
  //float pW = parseFloat(values[1]);
  //float pF = parseFloat(values[2]);
  //float pS = parseFloat(values[3]);
  //int duration = parseInt(values[4]);

  //sendTENSTrigger(id, pW, pF, pS, duration);
}

void printLog(String s) {
  connectionTextArea.setText(giveDateStr() + " " + s + "\n" + connectionTextArea.getText());
}

void keyReleased() {
  switch (key) {
    // channel 1
  case '1':
    println("released '1'");
    sendTENSTrigger(1, 1f, .5f, .3f, 400);
    break;
  case 'q':
    println("released 'q'");
    sendTENSTrigger(1, .66f, .5f, .3f, 400);
    break;
  case 'a':
    println("released 'a'");
    sendTENSTrigger(1, .33f, .5f, .3f, 400);
    break;
  case 'z':
    println("released 'z'");
    sendTENSTrigger(1, 0f, .5f, .3f, 400);
    break;

    // channel 2
  case '2':
    println("released '2'");
    sendTENSTrigger(2, .6f, 1f, .3f, 450);
    break;
  case 'w':
    println("released 'w'");
    sendTENSTrigger(2, .6f, .66f, .3f, 450);
    break;
  case 's':
    println("released 's'");
    sendTENSTrigger(2, .6f, .33f, .3f, 450);
    break;
  case 'x':
    println("released 'x'");
    sendTENSTrigger(2, .6f, 0f, .3f, 450);
    break;

    // channel 3
  case '3':
    println("released '3'");
    sendTENSDoubleTrigger(
      3, 1f, .7f, .3f, 1500,
      2, 1f, .7f, .3f, 1500
      );
    break;
  case 'e':
    println("released 'e'");
    sendTENSDoubleTrigger(
      3, .66f, .6f, .3f, 1100,
      4, .66f, .6f, .3f, 1100
      );
    break;
  case 'd':
    println("released 'd'");
    sendTENSDoubleTrigger(
      3, .33f, .5f, .3f, 800,
      5, .33f, .5f, .3f, 800
      );
    break;
  case 'c':
    println("released 'c'");
    sendTENSDoubleTrigger(
      3, 0f, .4f, .3f, 500,
      6, 0f, .4f, .3f, 500
      );
    break;

    // channel 4
  case '4':
    println("released '4'");
    sendTENSDoubleTrigger(
      4, .7f, 1f, .3f, 1500,
      7, .7f, 1f, .3f, 1500
      );
    break;
  case 'r':
    println("released 'r'");
    sendTENSDoubleTrigger(
      4, .6f, .7f, .3f, 1100,
      8, .6f, .7f, .3f, 1100
      );
    break;
  case 'f':
    println("released 'f'");
    sendTENSDoubleTrigger(
      4, .5f, .5f, .3f, 800,
      7, .5f, .5f, .3f, 800
      );
    break;
  case 'v':
    println("released 'v'");
    sendTENSDoubleTrigger(
      4, .4f, .2f, .3f, 500,
      8, .4f, .2f, .3f, 500
      );
    break;

  case ' ':
    println("released ' '");
    connectionTextArea.setText(giveDateStr() + " " + "HENK\n" + connectionTextArea.getText());
    break;
  }
}

String addZeroes(int in) {
  return (in < 10) ? "0" + in : "" + in;
}
String giveDateStr() {
  return addZeroes(hour()) + ":" + addZeroes(minute()) + ":" + addZeroes(second());
}


void sendTENSTrigger(int id_1, float pulseWidth_1, float pulseFrequency_1, float strength_1, int duration_1) {
  OscMessage myMessage = new OscMessage("/trigger");

  myMessage.add(id_1); /* add an int to the osc message */
  myMessage.add(pulseWidth_1);
  myMessage.add(pulseFrequency_1);
  myMessage.add(strength_1);
  myMessage.add(duration_1);

  //println("sending " + myMessage.toString());

  /* send the message */
  oscP5.send(myMessage, oscRemoteESP32);
}


void sendOSCTriggerFeedbackToTD(int success, int id1) {
  sendOSCTriggerFeedbackToTD(success, id1, -1);
}
void sendOSCTriggerFeedbackToTD(int success, int id1, int id2) {
  if (!initOSCToTDInitted) {
    println("ERROR >> osc connection to TD is not initted yet");
    return;
  }

  OscMessage myMessage = new OscMessage("/trigger");

  myMessage.add(success); /* add an int to the osc message */
  myMessage.add(id1);
  if (id2 != -1) {
    myMessage.add(id2);
  }

  oscP5.send(myMessage, oscRemoteTD);
}

void sendTENSDoubleTrigger(
  int id_1, float pulseWidth_1, float pulseFrequency_1, float strength_1, int duration_1,
  int id_2, float pulseWidth_2, float pulseFrequency_2, float strength_2, int duration_2
  ) {
  OscMessage myMessage = new OscMessage("/trigger");

  // trigger 1
  myMessage.add(id_1); /* add an int to the osc message */
  myMessage.add(pulseWidth_1);
  myMessage.add(pulseFrequency_1);
  myMessage.add(strength_1);
  myMessage.add(duration_1);

  // trigger 2
  myMessage.add(id_2); /* add an int to the osc message */
  myMessage.add(pulseWidth_2);
  myMessage.add(pulseFrequency_2);
  myMessage.add(strength_2);
  myMessage.add(duration_2);

  //println("sending " + myMessage.toString());

  /* send the message */
  oscP5.send(myMessage, oscRemoteESP32);
}

void sendHeartbeat() {
  OscMessage myMessage = new OscMessage("/heartbeat");

  //println((millis() / 1000.0) + " >> sending osc heartbeat");

  oscP5.send(myMessage, oscRemoteESP32);
}


/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  //print("### received an osc message.");
  //print(" addrpattern: "+theOscMessage.addrPattern());
  //println(" typetag: "+theOscMessage.typetag());

  // comes from ESP32
  if (theOscMessage.addrPattern().equals("/heartbeat")) {
    if (isReceivingHeartbeat == false) {
      printLog("HEARTBEAT - connection established");
      isReceivingHeartbeat = true;
    }
    lastHeartBeat_timestamp = millis();
  }

  // comes from ESP32
  if (theOscMessage.addrPattern().equals("/trigger")) {
    //println("got osc message /trigger with typetag '" + theOscMessage.typetag() + "'");
    if (theOscMessage.typetag().equals("ii")) {

      if (theOscMessage.get(0).intValue() == 1) {
        printLog("ESP32 >> TRIGGER >> id: " + theOscMessage.get(1).intValue());
      } else {
        printLog("ESP32 >> TRIGGER >> FAILED >> id: " + theOscMessage.get(1).intValue());
      }

      sendOSCTriggerFeedbackToTD(theOscMessage.get(0).intValue(), theOscMessage.get(1).intValue());
    }
    if (theOscMessage.typetag().equals("iii")) {
      //println("OSC >> /trigger >> received two trigger id's");

      if (theOscMessage.get(0).intValue() == 1) {
        printLog("ESP32 >> DOUBLE TRIGGER >> id 1: " + theOscMessage.get(1).intValue() + ", id 2: " + theOscMessage.get(2).intValue());
      } else {
        printLog("ESP32 >> DOUBLE TRIGGER >> FAILED >> id 1: " + theOscMessage.get(1).intValue() + ", id 2: " + theOscMessage.get(2).intValue());
      }

      sendOSCTriggerFeedbackToTD(theOscMessage.get(0).intValue(), theOscMessage.get(1).intValue(), theOscMessage.get(2).intValue());
    }
  }

  // comes from ESP32
  if (theOscMessage.addrPattern().equals("/debugmsg")) {
    if (theOscMessage.typetag().equals("s")) {
      //println("RECEIVED a valid OSC DEBUG MESSAGE");
      //println("remote debug msg: " + theOscMessage.get(0));
      printLog("ESP32 >> " + theOscMessage.get(0).stringValue());
    }
  }
}

/**
 * To perform any action on datagram reception, you need to implement this
 * handler in your code. This method will be automatically called by the UDP
 * object each time he receive a nonnull message.
 * By default, this method have just one argument (the received message as
 * byte[] array), but in addition, two arguments (representing in order the
 * sender IP address and his port) can be set like below.
 */
// void receive( byte[] data ) {       // <-- default handler
void receive( byte[] data, String ip, int port ) {  // <-- extended handler

  if (oscTDTargetIP.equals("")) {
    oscTDTargetIP = ip;
    initOSCToTD();
  }
  String message = new String( data );
  tryParseAndSendThroughOSC(message);

  //println( "receive: \""+message+"\" from "+ip+" on port "+port );
}
