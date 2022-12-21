/*

 *** OSC protocol
 Packet of 5 arguments: ifffi
 int: ID of elektrode pair to trigger (1-21) (21 elektrode pairs, no?)
 float: pulse width (0.0-1.0)
 float: pulse frequency (0.0-1.0)
 float: pulse strength (0.0-1.0)
 int: pulse duration
 
 
 ***
 v02 -> send commands for four elektrodes
 
 
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
  myRemoteLocation = new NetAddress("192.168.1.201", 54321);

  frameRate(30);
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
    sendTENSTrigger(3, 1f, .7f, .3f, 500);
    break;
  case 'e':
    println("released 'e'");
    sendTENSTrigger(3, .66f, .6f, .3f, 500);
    break;
  case 'd':
    println("released 'd'");
    sendTENSTrigger(3, .33f, .5f, .3f, 500);
    break;
  case 'c':
    println("released 'c'");
    sendTENSTrigger(3, 0f, .4f, .3f, 500);
    break;

    // channel 4
  case '4':
    println("released '4'");
    sendTENSTrigger(4, .7f, 1f, .3f, 500);
    break;
  case 'r':
    println("released 'r'");
    sendTENSTrigger(4, .6f, .7f, .3f, 500);
    break;
  case 'f':
    println("released 'f'");
    sendTENSTrigger(4, .5f, .5f, .3f, 500);
    break;
  case 'v':
    println("released 'v'");
    sendTENSTrigger(4, .4f, .2f, .3f, 500);
    break;

}
}

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

  //println((millis() / 1000.0) + " >> sending osc heartbeat");

  oscP5.send(myMessage, myRemoteLocation);
}


/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  println(" typetag: "+theOscMessage.typetag());
}
