
void initTENSIDToBoardPortMapping() {

  // FYI -> IdToBoardPort(0, 1);

  // board 1 -> weaker power setting -> id's 1, 2, 3, 4, 5, 16, 17, 18, 19, 20
  id_to_boardport[1]  = IdToBoardPort(0, 1);
  id_to_boardport[16] = IdToBoardPort(0, 2);
  id_to_boardport[4]  = IdToBoardPort(0, 3);
  id_to_boardport[2]  = IdToBoardPort(0, 4);
  id_to_boardport[17] = IdToBoardPort(0, 5);
  id_to_boardport[19] = IdToBoardPort(0, 6);
  id_to_boardport[18] = IdToBoardPort(0, 7);
  id_to_boardport[20] = IdToBoardPort(0, 8);
  id_to_boardport[3]  = IdToBoardPort(0, 9);
  id_to_boardport[5]  = IdToBoardPort(0, 10);
  id_to_boardport[11] = IdToBoardPort(0, 11);
  id_to_boardport[9]  = IdToBoardPort(0, 12);

  // board 2 -> stronger power setting -> id's 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 21
  id_to_boardport[8]  = IdToBoardPort(1, 1);
  id_to_boardport[13] = IdToBoardPort(1, 2);
  id_to_boardport[7]  = IdToBoardPort(1, 3);
  id_to_boardport[14] = IdToBoardPort(1, 4);
  id_to_boardport[12] = IdToBoardPort(1, 5);
  id_to_boardport[10] = IdToBoardPort(1, 6);
  id_to_boardport[6]  = IdToBoardPort(1, 7);
  id_to_boardport[15] = IdToBoardPort(1, 8);
  id_to_boardport[21] = IdToBoardPort(1, 9);

  //  id_to_boardport[9] = IdToBoardPort(1, 1);
  //  id_to_boardport[10] = IdToBoardPort(1, 2);
  //  id_to_boardport[11] = IdToBoardPort(1, 3);
  //  id_to_boardport[12] = IdToBoardPort(1, 4);
  //  id_to_boardport[13] = IdToBoardPort(1, 5);
  //  id_to_boardport[14] = IdToBoardPort(1, 6);
  //  id_to_boardport[15] = IdToBoardPort(1, 7);
  //  id_to_boardport[16] = IdToBoardPort(1, 8);

  /*
    id_to_boardport[1] = IdToBoardPort(0, 15);
    id_to_boardport[2] = IdToBoardPort(0, 16);
    id_to_boardport[3] = IdToBoardPort(0, 4);
    id_to_boardport[4] = IdToBoardPort(0, 5);
    id_to_boardport[5] = IdToBoardPort(0, 7);
    id_to_boardport[6] = IdToBoardPort(0, 6);
    id_to_boardport[7] = IdToBoardPort(0, 11);
    id_to_boardport[8] = IdToBoardPort(0, 9);
    id_to_boardport[9] = IdToBoardPort(0, 8);
    id_to_boardport[10] = IdToBoardPort(0, 12);
    id_to_boardport[11] = IdToBoardPort(0, 13);
    id_to_boardport[12] = IdToBoardPort(1, 15);
    id_to_boardport[13] = IdToBoardPort(1, 5);
    id_to_boardport[14] = IdToBoardPort(1, 2);
    id_to_boardport[15] = IdToBoardPort(1, 16);
    id_to_boardport[16] = IdToBoardPort(1, 1);
    id_to_boardport[17] = IdToBoardPort(2, 5);
    id_to_boardport[18] = IdToBoardPort(2, 7);
    id_to_boardport[19] = IdToBoardPort(2, 9);
    id_to_boardport[20] = IdToBoardPort(2, 11);
    id_to_boardport[21] = IdToBoardPort(2, 2);
  */
}

void initMuxBoards() {

  pinMode(mux_1_s0, OUTPUT);
  pinMode(mux_1_s1, OUTPUT);
  pinMode(mux_1_s2, OUTPUT);
  pinMode(mux_1_s3, OUTPUT);
  digitalWrite(mux_1_s0, LOW);
  digitalWrite(mux_1_s1, LOW);
  digitalWrite(mux_1_s2, LOW);
  digitalWrite(mux_1_s3, LOW);

  pinMode(mux_2_s0, OUTPUT);
  pinMode(mux_2_s1, OUTPUT);
  pinMode(mux_2_s2, OUTPUT);
  pinMode(mux_2_s3, OUTPUT);
  digitalWrite(mux_2_s0, LOW);
  digitalWrite(mux_2_s1, LOW);
  digitalWrite(mux_2_s2, LOW);
  digitalWrite(mux_2_s3, LOW);

  //  pinMode(mux_3_s0, OUTPUT);
  //  pinMode(mux_3_s1, OUTPUT);
  //  pinMode(mux_3_s2, OUTPUT);
  //  pinMode(mux_3_s3, OUTPUT);
  //  digitalWrite(mux_3_s0, LOW);
  //  digitalWrite(mux_3_s1, LOW);
  //  digitalWrite(mux_3_s2, LOW);
  //  digitalWrite(mux_3_s3, LOW);

  setMux_1(MUX_OFF_PIN);
  setMux_2(MUX_OFF_PIN);
  //  setMux_3(MUX_OFF_PIN);

}


void allMuxesOff() {
  setMux_1(MUX_OFF_PIN);
  setMux_2(MUX_OFF_PIN);
  //  setMux_3(MUX_OFF_PIN);

}

void setMux_1(int channel) {
  int controlPin[] = {mux_1_s0, mux_1_s1, mux_1_s2, mux_1_s3};

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

}

void setMux_2(int channel) {
  int controlPin[] = {mux_2_s0, mux_2_s1, mux_2_s2, mux_2_s3};

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

}

void setMux_3(int channel) {
  int controlPin[] = {mux_3_s0, mux_3_s1, mux_3_s2, mux_3_s3};

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

}
