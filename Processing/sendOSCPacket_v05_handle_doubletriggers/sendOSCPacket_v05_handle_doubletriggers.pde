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
 
 
 */

import hypermedia.net.*;
import oscP5.*;
import netP5.*;

UDP udp;  // define the UDP object

OscP5 oscP5;
NetAddress myRemoteLocation;

void setup() {
  size(400, 400);

  initOSC();

  initUDP();

  frameRate(30);
}

void initOSC() {
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this, 7075);

  /* myRemoteLocation is a NetAddress. a NetAddress takes 2 parameters,
   * an ip address and a port number. myRemoteLocation is used as parameter in
   * oscP5.send() when sending osc packets to another computer, device,
   * application. usage see below. for testing purposes the listening port
   * and the port of the remote location address are the same, hence you will
   * send messages back to this sketch.
   */
  // @home : 192.168.1.201
  // @theater : 192.168.72.201
  // @theater - production AP : 10.0.0.88
  myRemoteLocation = new NetAddress("10.0.0.88", 54321);
}

void initUDP() {
  // create a new datagram connection on port 6000
  // and wait for incomming message
  udp = new UDP( this, 7001 );
  //udp.log( true );     // <-- printout the connection activity
  udp.listen( true );
}


void draw() {
  background(0);
  fill(255);
  text("keys z,a,q,1 to control elektrode pair #1 in increasing intensities", 15, 20);
  text("keys x,s,w,2 to control elektrode pair #2 in increasing intensities", 15, 40);
  text("keys c,d,e,3 to control elektrode pair #3 in increasing intensities", 15, 60);
  text("keys v,f,r,4 to control elektrode pair #4 in increasing intensities", 15, 80);

  /*
  fill(255, 64);
   if (mousePressed) {
   fill(0, 128, 0, 64);
   sendTENSTrigger(
   (int)random(1, 21),
   map(mouseX, 0, width, 0.0, 1.0),
   map(mouseY, height, 0, 0.0, 1.0),
   random(0.0, 0.2),
   100
   );
   }
   rect(0, 0, mouseX, height);
   rect(0, mouseY, width, height - mouseY);
   //*/


  if (frameCount % 10 == 0) {
    sendHeartbeat();
  }
}

void tryParseAndSendThroughOSC(String message) {
  message = message.trim();

  String[] values = split(message, "  ");
  //println("f:tryParseAndSendThroughOSC >> picked apart example packet into " + values.length + " parts");

  if (values.length != 5 && values.length != 10) {
    println("f:tryParseAndSendThroughOSC >> message did not contain 5 or 10 (double-trigger) arguments >> skipping");
    return;
  }

  if (values.length == 5) {
    sendTENSTrigger(
      parseInt(values[0]), parseFloat(values[1]), parseFloat(values[2]), parseFloat(values[3]), parseInt(values[4])
      );
  }
  if (values.length == 10) {
    sendTENSDoubleTrigger(
      parseInt(values[0]), parseFloat(values[1]), parseFloat(values[2]), parseFloat(values[3]), parseInt(values[4]),
      parseInt(values[5]), parseFloat(values[6]), parseFloat(values[7]), parseFloat(values[8]), parseInt(values[9])
      );
  }

  //int id = parseInt(values[0]);
  //float pW = parseFloat(values[1]);
  //float pF = parseFloat(values[2]);
  //float pS = parseFloat(values[3]);
  //int duration = parseInt(values[4]);

  //sendTENSTrigger(id, pW, pF, pS, duration);
}


void mousePressed() {
  /*
  
   OscMessage myMessage = new OscMessage("/trigger");
   
   myMessage.add((int)random(0, 21));
   myMessage.add(random(0.0, 1.0));
   myMessage.add(random(0.0, 1.0));
   myMessage.add(random(0.0, 1.0));
   
   println("sending " + myMessage.toString());
   
   oscP5.send(myMessage, myRemoteLocation);
   //*/
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
  }
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
  oscP5.send(myMessage, myRemoteLocation);
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
  oscP5.send(myMessage, myRemoteLocation);
}

void sendHeartbeat() {
  OscMessage myMessage = new OscMessage("/heartbeat");

  //println((millis() / 1000.0) + " >> sending osc heartbeat");

  oscP5.send(myMessage, myRemoteLocation);
}


/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  //print("### received an osc message.");
  //print(" addrpattern: "+theOscMessage.addrPattern());
  //println(" typetag: "+theOscMessage.typetag());

  if (theOscMessage.addrPattern().equals("/debugmsg")) {
    if (theOscMessage.typetag().equals("s")) {
      //println("RECEIVED a valid OSC DEBUG MESSAGE");
      println("remote debug msg: " + theOscMessage.get(0));
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

  String message = new String( data );
  tryParseAndSendThroughOSC(message);

  //println( "receive: \""+message+"\" from "+ip+" on port "+port );
}
