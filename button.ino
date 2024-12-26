#include "M5Dial.h"
#include "utility/M5Timer.h"
#include "Unit_4RELAY.h"


static long oldPosition = -999;
static long newPosition = 10;
static long backupPosition = 10;
static bool BtnL = false;
static bool BtnR = true;
static bool Relay1 = false;
static bool Relay2 = false;
static m5::touch_state_t prev_state;
M5Timer CountDown;
static bool Running = false;
static bool Pause = false;
uint16_t timerColor = BLACK;
UNIT_4RELAY relay;
int outV = 0;
int outL = 1;
int outU = 2;
bool zehnfach = false;


void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(bottom_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);

    // SDA, SCL  Red: 13, 15 Black: 2,1
    relay.begin(&Wire,13,15);
    relay.Init(1);

    drawButton(BtnL,Relay1);
    drawButton(BtnR,Relay2);

    CountDown.setInterval(1000, oneSecond);

    M5Dial.Encoder.write(10);
}

void drawTime(int ti, uint16_t c){
  M5Dial.Display.fillRect(0,0,M5Dial.Display.width(),M5Dial.Display.height()/2-1, c);
  M5Dial.Display.drawString(String(ti), M5Dial.Display.width() / 2, M5Dial.Display.height() *3/7);
  if (Running){
    relay.relayWrite(2, 1);
  }
  else{
    relay.relayWrite(2, 0);
  }
}

void oneSecond(){
  if (Running){
     M5Dial.Display.fillRect(0,0,M5Dial.Display.width(),M5Dial.Display.height()/2-1, BLACK );
      newPosition = newPosition - 1;
      if (newPosition == 0){
        Running = false;
        timerColor = BLACK;
        newPosition = backupPosition;
        Relay1 = 0;
        drawButton(BtnL, Relay1);
        M5Dial.Speaker.tone(1800, 500);
      }      
      oldPosition = newPosition;
      M5Dial.Encoder.write(newPosition);
      drawTime(newPosition, timerColor);
      M5Dial.Speaker.tone(8000, 20);
  }
 
}

void drawButton(bool Btn, bool State){
    uint16_t c = RED;
    if (State) c = GREEN;

    if (!Btn){
      M5Dial.Display.fillRect(0,M5Dial.Display.height()/2 + 4,M5Dial.Display.width()/2-2 ,M5Dial.Display.height(), c );
      M5Dial.Display.drawString("V", M5Dial.Display.width() / 4, M5Dial.Display.height()*7/8);
      relay.relayWrite(0, State);
    }
    else{
      M5Dial.Display.fillRect(M5Dial.Display.width()/2+2,M5Dial.Display.height()/2 + 4 ,M5Dial.Display.width() ,M5Dial.Display.height(), c );
      M5Dial.Display.drawString("L",M5Dial.Display.width()*3/4, M5Dial.Display.height()*7/8);
      relay.relayWrite(1, State);
    }
}

void readTouch(){
  auto t = M5Dial.Touch.getDetail();
  if (prev_state != t.state) {
    prev_state = t.state;

    if (t.y < M5Dial.Display.height()/2){
      if (t.wasPressed()){
        M5Dial.Speaker.tone(8000, 20);
        if (Running){
          Running = false;
          Pause = true;
          timerColor = LIGHTGREY;
          drawTime(newPosition, timerColor);
        }
        else if (Pause){
          Pause = false;
          timerColor = BLUE;
          CountDown.restartTimer(0);
          Running = true;
        }
        else{
          timerColor = BLUE;
          Running = true;
          CountDown.restartTimer(0);
          backupPosition = newPosition;
        }
        drawTime(newPosition, timerColor);
      }
      else if (t.wasHold()){
        Running = false;
        Pause = false;
        timerColor = BLACK;
        newPosition = backupPosition;
        oldPosition = newPosition;
        M5Dial.Encoder.write(newPosition);
        drawTime(newPosition, timerColor);
        M5Dial.Speaker.tone(1800, 500);
      }

    }
    else{
      if (t.wasPressed()){
        M5Dial.Speaker.tone(8000, 20);

        if (t.x < (M5Dial.Display.width()/2)){
          Relay1 = !Relay1;
          drawButton(BtnL, Relay1);
        }
        else
        {
          Relay2 = !Relay2;
          drawButton(BtnR, Relay2);
        }
      }
    }
    
  }
}


void loop() {
    M5Dial.update();
    CountDown.run();
    readTouch();
    newPosition = M5Dial.Encoder.read();
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        if (zehnfach){
          newPosition = oldPosition + ((newPosition-oldPosition)*10);
          M5Dial.Encoder.write(newPosition);
        }
        if (newPosition < 1){
          newPosition = 1;
          M5Dial.Encoder.write(1);
        }
        oldPosition = newPosition;
        drawTime(newPosition, timerColor);
    }
    if (M5Dial.BtnA.wasPressed()) zehnfach = !zehnfach;
    
}