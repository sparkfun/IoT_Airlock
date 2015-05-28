//Stuff for finger print scanner:
#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"

FPS_GT511C3 fps(4, 5);

//for easyVR communication:
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "SoftwareSerial.h"
  SoftwareSerial port(12,13);
#else // Arduino 0022 - use modified NewSoftSerial
  #include "WProgram.h"
  #include "NewSoftSerial.h"
  NewSoftSerial port(12,13);
#endif

//for RFID communication:
SoftwareSerial rSerial(2,3);

//all other pins used for LEDs and checking:
int errorPin = 6;
int fingerPinGood = 9;
int RFIDcheckPinGood = 10;
int passwordPinGood = 11;
int triesLeft = 5;
int IDpin1 = 7;
int IDpin2 = 8;
int doorCheckPin = A0;
int doorStatus;

//some variables for keeping track of what's been done:
int countDown;
int fingerSearch = 0;
int checkRFID = 0;

//some variables for the RFID scanner:
const int tagLen = 16;
const int idLen = 13;
const int kTags = 1;

//List of known/accepted RFIDs:
char knownTags[kTags][idLen] = {
             "6A0049FC38E7"

};

//More stuff for RFID scanner:
char newTag[idLen];

#include "EasyVR.h"
EasyVR easyvr(port);

//Groups and Commands for easyVR:
enum Groups
{
  GROUP_1  = 1,
};

enum Group1 
{
  G1_SARAH_IS_AWESOME = 0,
  G1_SARAH_IS_AWESOME_SHAWN = 1,
  G1_SARAH_IS_AWESEOM_NICK = 2,
};


EasyVRBridge bridge;

int8_t group, idx;

void setup()
{
  // bridge mode? 
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  // run normally
  Serial.begin(9600); //for checking what's going on
  port.begin(9600);   //for talking to the easyVR
  rSerial.begin(9600);//for talking to the RFID scanner
  delay(100);
  //initialize finger print scanner stuff:
  fps.Open();
  fps.SetLED(true);

  //set up the serial port to talk to the easyVR to check status:
  port.listen();
  if (!easyvr.detect())
  {
    Serial.println("EasyVR not detected!");
    for (;;);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  Serial.println("EasyVR detected!");
  easyvr.setTimeout(5);
  easyvr.setLanguage(0);

  //start easyVR in group 1 so we don't need a trigger word, and check that:
  group = 1; //<-- start group (customize)
   Serial.print("RFID hooked up");
   Serial.println();
   
   //set up all pins and all that good jazz:
   pinMode(errorPin, OUTPUT);
   pinMode(RFIDcheckPinGood, OUTPUT);
   pinMode(passwordPinGood, OUTPUT);
   pinMode(fingerPinGood, OUTPUT);
   pinMode(IDpin1, OUTPUT);
   pinMode(IDpin2, OUTPUT);
   pinMode(doorCheckPin, INPUT);
   digitalWrite(errorPin, HIGH);
   digitalWrite(RFIDcheckPinGood, HIGH);
   digitalWrite(passwordPinGood, HIGH);
   digitalWrite(fingerPinGood, HIGH);
   digitalWrite(IDpin1, LOW);
   digitalWrite(IDpin2, LOW);
}

void action();


void loop(){
  //if the finger print scanner and the RFID have successfully 
  //been passed; now do the easyVR password:
  if ((fingerSearch == 1) && (checkRFID == 1)){
    countDown = 5;
    while (countDown > 0){
      port.listen(); //listening to the easyVR shield now
      easyvr.setPinOutput(EasyVR::IO1, HIGH); //green LED on board (listening)
                              
      Serial.print("Say password in Group ");
      Serial.println(group);
      easyvr.recognizeCommand(group);
                              
      do
      {
      // can do some processing while waiting for a spoken command
      }
      while (!easyvr.hasFinished());
                                
      easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off
       
      //not really using a trigger but there for kicks:                       
      idx = easyvr.getWord();
      if (idx >= 0)
      {
      // built-in trigger (ROBOT)
      // group = GROUP_X; <-- jump to another group X
         return;
      }
      //listening for stuff:
      idx = easyvr.getCommand();
      if (idx >= 0)
      {
      // print debug message
        uint8_t train = 0;
        char name[32];
        Serial.print("Command: ");
        Serial.print(idx);
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          Serial.print(" = ");
          Serial.println(name);
        }
        else
          Serial.println();
          easyvr.playSound(0, EasyVR::VOL_FULL);
          // perform some action
          action();
        }
        else // errors or timeout
        {
          //times out every like 10 secounds because who knows why:
          if (easyvr.isTimeout())
            Serial.println("Timed out, try again...");
            int16_t err = easyvr.getError();
            //in case we want to keep track of tries left (start at 5):
            countDown = countDown - 1;
            //treating a time running out as a failed attempt:
            digitalWrite(errorPin, LOW);
            delay(500);
            digitalWrite(errorPin, HIGH);
            if (err >= 0)
            {
              Serial.print("Error ");
              Serial.println(err, HEX);
            }
         }
      }
  }
  //if the finger scanner has been successfully passed but not
  //the RFID scan then do the RFID scan stuff:
  else if ((fingerSearch == 1) && (checkRFID == 0)){
    Serial.print("Scan RFID ");
    Serial.println();
    rSerial.listen();
    // Counter for the newTag array
    int i = 0;
    // Variable to hold each byte read from the serial buffer
    int readByte;
    // Flag so we know when a tag is over
    boolean tag = false;
                        
    // This makes sure the whole tag is in the serial buffer before
    // reading, the Arduino can read faster than the ID module can deliver!
    if (rSerial.available() == tagLen) {
      tag = true;
    }
                        
    if (tag == true) {
      while (rSerial.available()) {
        // Take each byte out of the serial buffer, one at a time
        readByte = rSerial.read();
                          
        /* This will skip the first byte (2, STX, start of text) and the last three,
        ASCII 13, CR/carriage return, ASCII 10, LF/linefeed, and ASCII 3, ETX/end of 
        text, leaving only the unique part of the tag string. It puts the byte into
        the first space in the array, then steps ahead one spot */
        if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
          newTag[i] = readByte;
          i++;
        }
                          
        // If we see ASCII 3, ETX, the tag is over
        if (readByte == 3) {
          tag = false;
        }
                        
      }
    }
                        
                        
    // don't do anything if the newTag array is full of zeroes
    if (strlen(newTag)== 0) {
      return;
    }
                        
    else {
      int total = 0;
                        
      for (int ct=0; ct < kTags; ct++){
        total = checkTag(newTag, knownTags[ct]);
      }
                        
      // If newTag matched any of the tags
      // we checked against, total will be 1
      if (total > 0) {
                        
      // Put the action of your choice here!
                        
      // I'm going to rotate the servo to symbolize unlocking the lockbox
                        
        Serial.println("Success!");
        digitalWrite(RFIDcheckPinGood, LOW);
        digitalWrite(fingerPinGood, HIGH);
        checkRFID = 1;
      }
      else {
      // This prints out unknown cards so you can add them to your knownTags as needed
        //if we want to track these tries left (starts at 5):
        triesLeft = triesLeft - 1;
        //unknown tags count as failures:
        Serial.print("Unknown tag! ");
        Serial.print(newTag);
        Serial.println();
        Serial.print("Try again, ");
        Serial.print(triesLeft);
        Serial.print(" tries left.");
        Serial.println();
        digitalWrite(errorPin, LOW);
        delay(500);
        digitalWrite(errorPin, HIGH);
        
                                
      }
                            
                            
                            
    }
                        
    // Once newTag has been checked, fill it with zeroes
    // to get ready for the next tag read
    for (int c=0; c < idLen; c++) {
      newTag[c] = 0;
    }
                          
    if (triesLeft <= 0){
    //Do something terrible because shame on you for failing 3 times
      triesLeft = 5;
    }
  }
  //if the finger print scanner hasn't been passed yet then do that:
  else {
  // Identify fingerprint test
  if (fps.IsPressFinger())
  {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();
    if (id <200)
    {
      Serial.print("Verified ID:");
      Serial.println(id);
      //CHeck who's finger print:
      if (id == 0 | id == 1){
        digitalWrite(IDpin1, LOW);
        digitalWrite(IDpin2, HIGH);
        //Sarah
        Serial.print("Hello Sarah! ");
        Serial.println();
      }
      if (id == 2){
        digitalWrite(IDpin1, HIGH);
        digitalWrite(IDpin2, LOW);
        //Shawn
        Serial.print("Hello Shawn! ");
        Serial.println();
      }
      if (id == 3){
        digitalWrite(IDpin1, HIGH);
        digitalWrite(IDpin2, HIGH);
        //Nick
        Serial.print("Hello Nick! ");
        Serial.println();
      }
      digitalWrite(fingerPinGood, LOW);
      fingerSearch = 1;
    }
    else
    {
      //unidentified finger counts as a failure, no counter 
      //for this since it was the first one but whatever:
      Serial.println("Finger not found");
      digitalWrite(errorPin, LOW);
      delay(500);
      digitalWrite(errorPin, HIGH);
    }
  }
  else
  {
    Serial.println("Please press finger");
  }
  delay(100);
  }


  
  


}

//cases for easyVR groups, could have different passwords for 
//everyone, but I want all of you to have to keep saying that I
//am awesome:
void action()
{
    switch (group)
    {
    case GROUP_1:
      switch (idx)
      {
      case G1_SARAH_IS_AWESOME:
        // write your action code here
        Serial.print("Password confirmed, Sarah is awesome!");
        Serial.print(" ");
        digitalWrite(passwordPinGood, LOW);
        digitalWrite(RFIDcheckPinGood, HIGH);
        delay (500);
        //Check the inside door:
        doorStatus = digitalRead(doorCheckPin);
        //Resets after inside door is opened:
        if (doorCheckPin == HIGH){
          fingerSearch = 0;
          checkRFID = 0;
          digitalWrite(passwordPinGood, HIGH);
        }    
        break;
      case G1_SARAH_IS_AWESOME_SHAWN:
        // write your action code here
        Serial.print("Password confirmed, Sarah is awesome!");
        Serial.print(" ");
        digitalWrite(passwordPinGood, LOW);
        digitalWrite(RFIDcheckPinGood, HIGH);
        delay (500);
        //Check the inside door:
        doorStatus = digitalRead(doorCheckPin);
        //Resets after inside door is opened:
        if (doorCheckPin == HIGH){
          fingerSearch = 0;
          checkRFID = 0;
          digitalWrite(passwordPinGood, HIGH);
        } 
        break;
      case G1_SARAH_IS_AWESEOM_NICK:
        // write your action code here
        Serial.print("Password confirmed, Sarah is awesome!");
        Serial.print(" ");
        digitalWrite(passwordPinGood, LOW);
        digitalWrite(RFIDcheckPinGood, HIGH);
        delay (500);
        //Check the inside door:
        doorStatus = digitalRead(doorCheckPin);
        //Resets after inside door is opened:
        if (doorStatus == HIGH){
          fingerSearch = 0;
          checkRFID = 0;
          digitalWrite(passwordPinGood, HIGH);
        }    
        break;
      }
      break;
      
    }
    
}

//for RFID scanner checking scanned tag against relevant characters
//in the known tags:
int checkTag(char nTag[], char oTag[]) {
    for (int i = 0; i < idLen; i++) {
      if (nTag[i] != oTag[i]) {
        return 0;
      }
    }
  return 1;
}
