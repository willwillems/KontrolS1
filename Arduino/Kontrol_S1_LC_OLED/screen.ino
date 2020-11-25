void serialEvent() {
  while (Serial.available()) {
    byte type = Serial.read();
    Serial.print("type: ");
    Serial.println(type);
    byte infoLength = Serial.read();
    Serial.print("infoLength: ");
    Serial.println(infoLength);

    
    char info[50]; // make this dynamic
    for (int i = 0; i <= infoLength; i++) {
    // Serial.println(Serial.available());
    // for (int i = 0; Serial.available() > 0; i++) {
      info[i] = (char)Serial.read();
      Serial.print(info[i]);
    }
    Serial.println("");
    if (type == TRACK_TITLE) {
      setTrackName(info, true);
    } else if (type == TRACK_ARTIST) {
      setArtistName(info, true);
    } else if (type == TRACK_BPM) {
      // tempo info is a string
      setTempoValue(info, true);
    } else if (type == TRACK_PROGRESS) {
      trackProgress = atoi(info);
      setBarWidth((260 * trackProgress) / trackLength);
      generateTimeLeftString(trackLength - trackProgress);
    } else if (type == TRACK_TIME) {
      trackLength = atoi(info);
      setBarWidth(((screenWidth - 4) * trackProgress) / trackLength);
//  } else if (type == ) {
    } else {
      Serial.println("Invalid type");
    }
  }
}

void setTrackName (char *name, bool update) { // todo make update optional
  // clear area by drawing black box
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, (2*8), ((32-8)*8), (2*8));
  u8g2.setDrawColor(1);
  
  u8g2.setFont(u8g2_font_unifont_tf);  // choose a suitable font
  u8g2.drawStr(0,32, name);  // write something to the internal memoryv
  u8g2.sendBuffer();
  // if (update) { u8g2.updateDisplayArea(0, 2, (32-8), 3); }; // update relevant display section
}

void setArtistName (char *name, bool update) {
  // clear area by drawing black box
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, ((32-8)*8), (2*8));
  u8g2.setDrawColor(1);
  
  u8g2.setFont(u8g2_font_helvB14_tf);  // choose a suitable font
  u8g2.drawStr(0,14, name); // write something to the internal memory
  u8g2.sendBuffer();
  // if (update) { u8g2.updateDisplayArea(0, 0, (32-8), 2); }; // update relevant display section
}

void setTempoValue (char *tempo, bool update) {
  // clear area by drawing black box
  u8g2.setDrawColor(0);
  u8g2.drawBox(((32-8)*8), 0, (8*8), (2*8));
  u8g2.setDrawColor(1);
  
  u8g2.setFont(u8g2_font_helvB12_te); // u8g2_font_7x13B_mn, u8g2_font_profont17_mn
  u8g2.drawRBox(252-52, 0, 52, 17, 2);
  u8g2.setDrawColor(0);
  u8g2.drawStr(screenWidth - 2 - 52, 14, tempo);
  u8g2.setDrawColor(1);
  if (update) { u8g2.updateDisplayArea(32-8, 0, 8, 2); } //updateDisplayArea(32-4, 0, 4, 2); };
}

void setTrackTime (char *time, bool update) {
  // clear area by drawing black box
  u8g2.setDrawColor(0);
  u8g2.drawBox(((32-8)*8), (2*8), (8*8), (3*8));
  u8g2.setDrawColor(1);
  
  u8g2.setFont(u8g2_font_unifont_tf);
  u8g2.drawStr(screenWidth - 2 - 42, (14 + 18), time);
  if (update) { u8g2.updateDisplayArea(32-8, 2, 8, 3); };
}

void drawBarBox () {
  u8g2.drawBox(0,   (60-barHeight), 256,  1); // top
  u8g2.drawBox(0,   (64-1),         256,  1); // bottom
  u8g2.drawBox(0,   (60-barHeight), 1,    (4+barHeight)); // left
  u8g2.drawBox(255, (60-barHeight), 1,    (4+barHeight)); // right
}

void setBarWidth (int width) {
  // if == less than previous
  if (true) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(2, (62-barHeight), (screenWidth - 4), barHeight);
    u8g2.setDrawColor(1);
  }
  u8g2.drawBox(2, (62-barHeight), width, barHeight);
  u8g2.sendBuffer();
  // u8g2.updateDisplayArea(0, 5, 32, 3);
}

void generateTimeLeftString (int seconds) {
  const int _minutes = seconds / 60;
  const int _seconds = seconds % 60;
  trackTimeLeft[0] = '0' + _minutes / 10;
  trackTimeLeft[1] = '0' + _minutes % 10;
  trackTimeLeft[2] = ':';
  trackTimeLeft[3] = '0' + _seconds / 10;
  trackTimeLeft[4] = '0' + _seconds % 10;
  trackTimeLeft[5] = '\0';
  Serial.println(trackTimeLeft);
  setTrackTime(trackTimeLeft, true);
}
