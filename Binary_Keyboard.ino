#include <Keyboard.h>
#include <EEPROM.h>

byte buttonPins[2] = {0, 1}; //D0 = 0, D1 = 1
bool lastButtonState[2];

bool buttonPressed[2];

byte asciiValue;
uint8_t index;
uint8_t indexDir; //0 = left to right, 1 = right to left

byte dataPort[8] = {6, 7, 8, 9, 2, 3, 4, 5};
byte RS = 10;
byte RW = 11;
byte E = 12;

unsigned long pauseTime = 0;
bool pauseTimerStarted = false;
long pauseDelay = 2000; //2s

bool ignoreChangeMode = false;

byte contrastPin = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPins[0], INPUT_PULLUP);
  pinMode(buttonPins[1], INPUT_PULLUP);
  pinMode(contrastPin, OUTPUT);

  asciiValue = 0;

  if(EEPROM.read(1) != 0 && EEPROM.read(1) != 1)
    EEPROM.write(1, 0);
  else
    indexDir = EEPROM.read(1);
  index = 0 + (7 * indexDir);

  Init_LCD();
  resetScreen();

  analogWrite(contrastPin, 80);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(pauseTimerStarted) {
    if(pauseTime + pauseDelay < millis()) {
      resetScreen();
      pauseTimerStarted = false;
    }
  }
  
  readButtons();
  changeMode();

  for(int i = 0; i < 2; i++) {
    if(buttonPressed[i]) {
      asciiValue += i * round(pow(2, index));
      index += 1 + (-2 * indexDir);

      if(i)
        data(0x31);
      else
        data(0x30);

      if(index > 7 || index < 0) {
        bool displayAfter = 0;

        if(asciiValue >> 4 > 7) {
          writeString("INVALID!");

          pauseTime = millis();
          pauseTimerStarted = true;
        }
        else {
          writeString("WRITING");

          Keyboard.write(asciiValue);
          
          displayAfter = 1;
        }

        asciiValue = 0;
        index = 0 + (7 * indexDir);
        if(displayAfter)
          resetScreen();
      }

      buttonPressed[i] = 0;
    }
  }
}

void readButtons() {
  for(int i = 0; i < 2; i++) {
    int input = !digitalRead(buttonPins[i]);

    if(input != lastButtonState[i]) {

      delay(10);

      if(input != lastButtonState[i]) {
        if(!ignoreChangeMode)
          if(lastButtonState[i] && !input) {
            if(pauseTimerStarted) {
              resetScreen();
              pauseTimerStarted = false;
            }

            buttonPressed[i] = 1;
          }

        lastButtonState[i] = input;
      }
    }
  }
}

void changeMode() {
  if(!ignoreChangeMode) {
    if(lastButtonState[0] && lastButtonState[1]) {
      asciiValue = 0;
      
      if(indexDir) {
        EEPROM.write(1, 0);
        indexDir = 0;
        writeString("R-L MODE");
      }
      else {
        EEPROM.write(1, 1);
        indexDir = 1;
        writeString("L-R MODE");
      }
      index = 0 + (7 * indexDir);

      pauseTime = millis();
      pauseTimerStarted = true;

      ignoreChangeMode = true;
    }
  }
  else {
    if(!lastButtonState[0] && !lastButtonState[1])
      ignoreChangeMode = false;
  }
}

void resetScreen() {
  command(0x01);
  setShiftDir();
  command(0x0D);
}

void writeString(String str) {
  command(0x0C);
  command(0x06);
  command(0x01);

  command(0x80);
  for(int i = 0; i < str.length(); i++)
    data(char(str[i]));
}

void command(byte c){ //Function that sends commands
  for(int i = 0; i < 8; i++)
    digitalWrite(dataPort[i], bitRead(c, i));
   
  digitalWrite(RS, LOW);      //RS set to LOW for command
  digitalWrite(RW, LOW);      //R/W set to LOW  for writing
  digitalWrite(E, HIGH);     //E set to HIGH for latching
  delay(1);
  digitalWrite(E, LOW);       //E set to LOW for latching
}

void data(byte d){ //Function that sends data
  for(int i = 0; i < 8; i++)
    digitalWrite(dataPort[i], bitRead(d, i));
  
  digitalWrite(RS, HIGH);      //RS set to HIGH for data
  digitalWrite(RW, LOW);      //R/W set to LOW  for writing
  digitalWrite(E, HIGH);     //E set to HIGH for latching
  delay(1);
  digitalWrite(E, LOW);       //E set to LOW for latching
}

void Init_LCD() {
  pinMode(6, OUTPUT);         //Set DB0 as output
  pinMode(7, OUTPUT);         //Set DB1 as output
  pinMode(8, OUTPUT);         //Set DB2 as output
  pinMode(9, OUTPUT);         //Set DB3 as output
  pinMode(2, OUTPUT);         //Set DB4 as output
  pinMode(3, OUTPUT);         //Set DB5 as output
  pinMode(4, OUTPUT);         //Set DB6 as output
  pinMode(5, OUTPUT);         //Set DB7 as output
  pinMode(10, OUTPUT);        //Set RS  as output                
  pinMode(11, OUTPUT);        //Set RW  as output  
  pinMode(12, OUTPUT);        //Set E   as output

  digitalWrite(E, LOW);       //Set E  LOW
  delay(100);
  command(0x30);              //command 0x30 = Wake up
  delay(30); 
  command(0x30);              //command 0x30 = Wake up #2
  delay(10);
  command(0x30);              //command 0x30 = Wake up #3
  delay(10); 
  command(0x30);              //Function set: 8-bit/1-line/5x8-font
  command(0x0C);              //Display ON; Cursor/Blinking OFF
  setShiftDir();              //Entry mode set
}

void setShiftDir() {
  if(indexDir == 0) {         //Set cursor: 04 = Shift Right, 06 = Shift Left
    command(0x1C);
    command(0x04);
    command(0x87);
  }
  else {
    command(0x10);
    command(0x06);
    command(0x80);
  }
}