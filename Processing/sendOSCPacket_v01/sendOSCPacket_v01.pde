/**
 * oscP5sendreceive by andreas schlegel
 * example shows how to send and receive osc messages.
 * oscP5 website at http://www.sojamo.de/oscP5
 */

import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;

void setup() {
  size(400, 400);
  frameRate(25);
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this, 55555);

  /* myRemoteLocation is a NetAddress. a NetAddress takes 2 parameters,
   * an ip address and a port number. myRemoteLocation is used as parameter in
   * oscP5.send() when sending osc packets to another computer, device,
   * application. usage see below. for testing purposes the listening port
   * and the port of the remote location address are the same, hence you will
   * send messages back to this sketch.
   */
  // @home : 192.168.1.201
  // @theater : 192.168.72.201
  //myRemoteLocation = new NetAddress("192.168.1.201", 54321);
  myRemoteLocation = new NetAddress("192.168.105.8", 54321);

  frameRate(30);
}


void draw() {
  background(0);

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

  if (frameCount % 10 == 0) {
    sendHeartbeat();
  }
}

/*
void mousePressed() {
  OscMessage myMessage = new OscMessage("/trigger");

  myMessage.add((int)random(0, 21)); // add an int to the osc message
  myMessage.add(random(0.0, 1.0)); // add a float 
  myMessage.add(random(0.0, 1.0)); // add a float 
  myMessage.add(random(0.0, 1.0)); // add a float 

  println("sending " + myMessage.toString());

  oscP5.send(myMessage, myRemoteLocation);
}
//*/

void sendTENSTrigger(int id, float pulseWidth, float pulseFrequency, float strength, int duration) {
  OscMessage myMessage = new OscMessage("/trigger");

  myMessage.add(id); /* add an int to the osc message */
  myMessage.add(pulseWidth);
  myMessage.add(pulseFrequency);
  myMessage.add(strength);
  myMessage.add(duration);

  println("sending " + myMessage.toString());

  /* send the message */
  oscP5.send(myMessage, myRemoteLocation);
}

void sendHeartbeat() {
  OscMessage myMessage = new OscMessage("/heartbeat");

  //println("sending osc heartbeat");

  oscP5.send(myMessage, myRemoteLocation);
}


/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  println(" typetag: "+theOscMessage.typetag());
}
