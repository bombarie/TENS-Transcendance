/*

 *** OSC protocol
 Packet of 5 arguments: ifffi
 int: ID of elektrode pair to trigger (1-21) (21 elektrode pairs, no?)
 float: pulse width (0.0-1.0)
 float: pulse frequency (0.0-1.0)
 float: pulse strength (0.0-1.0)
 int: pulse duration
 
 
 ***
 v01
 
 The current separation of ID's over the boards:
 - board 1: 1, 2, 3, 4, 5, 9, 11, 16, 17, 18, 19, 20
 - board 2: 6, 7, 8, 10, 12, 13, 14, 15, 21
 
 ***
 v02
 
 Add double triggers. Double triggers are: 1+6, 16+15, 9+10, 11+12
 
 */

import hypermedia.net.*;
import oscP5.*;
import netP5.*;
import controlP5.*;

OscP5 oscP5;
NetAddress oscRemoteESP32;
NetAddress oscRemoteTD;

boolean initOSCToTDInitted = false;

ControlP5 cp5;

PFont font_andale;
PFont font_andale_12;


String oscTDTargetIP = "";
int oscTDTargetPort = 7002;

// @home : 192.168.1.201
// @theater : 192.168.72.201
// @theater - production AP : 10.0.0.88
String oscESP32TargetIP = "192.168.1.201";
int oscReceivePort = 7075;
int oscESP32TargetPort = 54321;

boolean isReceivingHeartbeat = false;
int lastHeartBeat_timestamp;
int heartbeatTimeout = 1500; // ms

// board 1 -> weaker power setting -> id's 1, 2, 3, 4, 5, 9, 11, 16, 17, 18, 19, 20
Button btn_id_01;
Button btn_id_02;
Button btn_id_03;
Button btn_id_04;
Button btn_id_05;
Button btn_id_09;
Button btn_id_11;
Button btn_id_16;
Button btn_id_17;
Button btn_id_18;
Button btn_id_19;
Button btn_id_20;

// board 2 -> stronger power setting -> id's 6, 7, 8, 10, 12, 13, 14, 15, 21
Button btn_id_06;
Button btn_id_07;
Button btn_id_08;
Button btn_id_10;
Button btn_id_12;
Button btn_id_13;
Button btn_id_14;
Button btn_id_15;
Button btn_id_21;

// double id triggers -> double triggers are 1+6, 16+15, 9+10, 11+12
Button btn_id_01_06;
Button btn_id_16_15;
Button btn_id_09_10;
Button btn_id_11_12;

float pulseWidth = 0.45;
float pulseFrequency = 0.55;

void setup() {
  size(850, 500);

  font_andale = loadFont("AndaleMono-16.vlw");
  font_andale_12 = loadFont("AndaleMono-12.vlw");
  textFont(font_andale, 13);
  textAlign(LEFT, TOP);

  initOSCToESP32();

  initControlP5();

  frameRate(30);

  printLog("started");
}

void draw() {
  background(#3b6662);
  fill(255);

  if (millis() - lastHeartBeat_timestamp > heartbeatTimeout) {
    if (isReceivingHeartbeat) {
      printLog("HEARTBEAT - connection lost");
      isReceivingHeartbeat = false;
    }
  }


  drawConnectionSettings(15, 20);
  drawHeartbeatConnection(15, 260);

  drawBoard01Triggers(275, 20);
  drawBoard02Triggers(450, 20);
  drawDoubleTriggers(625, 20);

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

void initControlP5() {
  cp5 = new ControlP5(this);

  // board 01
  btn_id_01 = cp5.addButton("trigger id 01").setValue(0).setSize(150, 19);
  btn_id_02 = cp5.addButton("trigger id 02").setValue(0).setSize(150, 19);
  btn_id_03 = cp5.addButton("trigger id 03").setValue(0).setSize(150, 19);
  btn_id_04 = cp5.addButton("trigger id 04").setValue(0).setSize(150, 19);
  btn_id_05 = cp5.addButton("trigger id 05").setValue(0).setSize(150, 19);
  btn_id_09 = cp5.addButton("trigger id 09").setValue(0).setSize(150, 19);
  btn_id_11 = cp5.addButton("trigger id 11").setValue(0).setSize(150, 19);
  btn_id_16 = cp5.addButton("trigger id 16").setValue(0).setSize(150, 19);
  btn_id_17 = cp5.addButton("trigger id 17").setValue(0).setSize(150, 19);
  btn_id_18 = cp5.addButton("trigger id 18").setValue(0).setSize(150, 19);
  btn_id_19 = cp5.addButton("trigger id 19").setValue(0).setSize(150, 19);
  btn_id_20 = cp5.addButton("trigger id 20").setValue(0).setSize(150, 19);

  // board 02
  btn_id_06 = cp5.addButton("trigger id 06").setValue(0).setSize(150, 19);
  btn_id_07 = cp5.addButton("trigger id 07").setValue(0).setSize(150, 19);
  btn_id_08 = cp5.addButton("trigger id 08").setValue(0).setSize(150, 19);
  btn_id_10 = cp5.addButton("trigger id 10").setValue(0).setSize(150, 19);
  btn_id_12 = cp5.addButton("trigger id 12").setValue(0).setSize(150, 19);
  btn_id_13 = cp5.addButton("trigger id 13").setValue(0).setSize(150, 19);
  btn_id_14 = cp5.addButton("trigger id 14").setValue(0).setSize(150, 19);
  btn_id_15 = cp5.addButton("trigger id 15").setValue(0).setSize(150, 19);
  btn_id_21 = cp5.addButton("trigger id 21").setValue(0).setSize(150, 19);

  btn_id_01_06 = cp5.addButton("trigger id 01-06").setValue(0).setSize(150, 19);
  btn_id_16_15 = cp5.addButton("trigger id 16-15").setValue(0).setSize(150, 19);
  btn_id_09_10 = cp5.addButton("trigger id 09-10").setValue(0).setSize(150, 19);
  btn_id_11_12 = cp5.addButton("trigger id 11-12").setValue(0).setSize(150, 19);
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

  text("*** OSC ( P5->TD )  ***", 0, 140);
  if (initOSCToTDInitted) {
    text("target ip: " + oscTDTargetIP, 0, 160);
    text("target port: " + oscTDTargetPort, 0, 180);
  } else {
    text("not initted (have not \nreceived a udp packet yet)", 0, 160);
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

void drawBoard01Triggers(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("Board 01 Triggers", 0, 0);
  btn_id_01.setPosition(x + 0, y + 25);
  btn_id_02.setPosition(x + 0, y + 50);
  btn_id_03.setPosition(x + 0, y + 75);
  btn_id_04.setPosition(x + 0, y + 100);
  btn_id_05.setPosition(x + 0, y + 125);
  btn_id_09.setPosition(x + 0, y + 150);
  btn_id_11.setPosition(x + 0, y + 175);
  btn_id_16.setPosition(x + 0, y + 200);
  btn_id_17.setPosition(x + 0, y + 225);
  btn_id_18.setPosition(x + 0, y + 250);
  btn_id_19.setPosition(x + 0, y + 275);
  btn_id_20.setPosition(x + 0, y + 300);

  popMatrix();
}

void drawBoard02Triggers(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("Board 02 Triggers", 0, 0);
  btn_id_06.setPosition(x + 0, y + 25);
  btn_id_07.setPosition(x + 0, y + 50);
  btn_id_08.setPosition(x + 0, y + 75);
  btn_id_10.setPosition(x + 0, y + 100);
  btn_id_12.setPosition(x + 0, y + 125);
  btn_id_13.setPosition(x + 0, y + 150);
  btn_id_14.setPosition(x + 0, y + 175);
  btn_id_15.setPosition(x + 0, y + 200);
  btn_id_21.setPosition(x + 0, y + 225);

  popMatrix();
}


void drawDoubleTriggers(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("Double ID Triggers", 0, 0);
  btn_id_01_06.setPosition(x + 0, y + 25);
  btn_id_16_15.setPosition(x + 0, y + 50);
  btn_id_09_10.setPosition(x + 0, y + 75);
  btn_id_11_12.setPosition(x + 0, y + 100);

  popMatrix();
}


void drawConnectionLog(int x, int y) {
  pushMatrix();
  translate(x, y);
  fill(255);

  text("COMMUNICATION LOG", 0, 0);

  popMatrix();
}

//void updateConnectionLog() {
//  int lines = split(connectionTextArea.getText(), "\n").length;

//  if (lines > 50) {
//    // find index of last carriage return
//    int lastCRLoc = connectionTextArea.getText().lastIndexOf("\n");

//    // shorten the textarea to that last index (ie: remove the last line)
//    connectionTextArea.setText(connectionTextArea.getText().substring(0, lastCRLoc));
//  }
//}

public void controlEvent(ControlEvent theEvent) {
  //println("controlEvent: " + theEvent.getController().getName());

  switch(theEvent.getController().getName()) {
  case "trigger id 01":
    sendTENSTrigger(1, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 02":
    sendTENSTrigger(2, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 03":
    sendTENSTrigger(3, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 04":
    sendTENSTrigger(4, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 05":
    sendTENSTrigger(5, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 16":
    sendTENSTrigger(16, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 17":
    sendTENSTrigger(17, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 18":
    sendTENSTrigger(18, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 19":
    sendTENSTrigger(19, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 20":
    sendTENSTrigger(20, pulseWidth, pulseFrequency, .3f, 850);
    break;


  case "trigger id 06":
    sendTENSTrigger(6, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 07":
    sendTENSTrigger(7, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 08":
    sendTENSTrigger(8, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 09":
    sendTENSTrigger(9, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 10":
    sendTENSTrigger(10, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 11":
    sendTENSTrigger(11, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 12":
    sendTENSTrigger(12, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 13":
    sendTENSTrigger(13, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 14":
    sendTENSTrigger(14, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 15":
    sendTENSTrigger(15, pulseWidth, pulseFrequency, .3f, 850);
    break;
  case "trigger id 21":
    sendTENSTrigger(21, pulseWidth, pulseFrequency, .3f, 850);
    break;


  case "trigger id 01-06":
    sendTENSDoubleTrigger(
      1, pulseWidth, pulseFrequency, .3f, 850,
      6, pulseWidth, pulseFrequency, .3f, 850
      );
    break;
  case "trigger id 16-15":
    sendTENSDoubleTrigger(
      16, pulseWidth, pulseFrequency, .3f, 850,
      15, pulseWidth, pulseFrequency, .3f, 850
      );
    break;
  case "trigger id 09-10":
    sendTENSDoubleTrigger(
      9, pulseWidth, pulseFrequency, .3f, 850,
      10, pulseWidth, pulseFrequency, .3f, 850
      );
    break;
  case "trigger id 11-12":
    sendTENSDoubleTrigger(
      11, pulseWidth, pulseFrequency, .3f, 850,
      12, pulseWidth, pulseFrequency, .3f, 850
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
  println("f:printLog >> " + s);
}

void keyReleased() {
  switch (key) {
    // channel 1
    //case '1':
    //  println("released '1'");
    //  sendTENSTrigger(1, 1f, .5f, .3f, 400);
    //  break;
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
    }
    if (theOscMessage.typetag().equals("iii")) {
      //println("OSC >> /trigger >> received two trigger id's");

      if (theOscMessage.get(0).intValue() == 1) {
        printLog("ESP32 >> DOUBLE TRIGGER >> id 1: " + theOscMessage.get(1).intValue() + ", id 2: " + theOscMessage.get(2).intValue());
      } else {
        printLog("ESP32 >> DOUBLE TRIGGER >> FAILED >> id 1: " + theOscMessage.get(1).intValue() + ", id 2: " + theOscMessage.get(2).intValue());
      }
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
