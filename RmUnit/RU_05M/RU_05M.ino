#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

const byte ERR_pin = 13;
String Grate_status, PIR5_status, TOUCHr_status, TOUCHl_status;
String NFC_UID[3] = {"", "", ""};
String NFC_REP[3] = {"NFC1=", "NFC2=", "NFC3="};
unsigned long prev_time = 0;
unsigned long curr_time = 0;
unsigned long delta_time = 0;
const byte PIR5_pin = A0;
const byte TOUCHr_pin = A1;
const byte TOUCHl_pin = A2;
const byte TPLedr_pin = A3;
const byte TPLedl_pin = A4;
const byte DLL_pin = 4;
const byte DLS_pin = 5;
const byte DLGH_pin = 6;
const byte DLFAC_pin = 7;
const byte DDc_pin = 8;
const byte DDo_pin = 9;
const byte Mon_pin = 11;
const byte Mdir_pin = 12;

#define BRIGHTNESS 50

int brightness = BRIGHTNESS;
int fadeAmount = 5;
#define WaitDel2 40
#define WaitDel3 60


byte mac[] = {0xD0, 0xF1, 0xC0, 0xA8, 0x02, 0x05};
IPAddress ip(192, 168, 2, 5);
IPAddress myDns(192, 168, 2, 10);
IPAddress gateway(192, 168, 2, 10);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server = EthernetServer(404);
byte ACT = 0;


void setup() {
  pinMode(ERR_pin, OUTPUT); //use 13 pin for error notification
  digitalWrite(ERR_pin, LOW);

  pinMode(PIR5_pin, INPUT);
  pinMode(TOUCHr_pin, INPUT_PULLUP);
  pinMode(TOUCHl_pin, INPUT_PULLUP);
  
  pinMode(TPLedr_pin, OUTPUT);
  pinMode(TPLedl_pin, OUTPUT);
  pinMode(DLL_pin, OUTPUT);
  pinMode(DLS_pin, OUTPUT);
  pinMode(DLGH_pin, OUTPUT);
  pinMode(DLFAC_pin, OUTPUT);
  digitalWrite(DLL_pin, HIGH);
  digitalWrite(DLS_pin, HIGH);
  digitalWrite(DLGH_pin, HIGH);
  digitalWrite(DLFAC_pin, HIGH);
  pinMode(DDc_pin, INPUT_PULLUP);
  pinMode(DDo_pin, INPUT_PULLUP);
  pinMode(Mon_pin, OUTPUT);
  pinMode(Mdir_pin, OUTPUT);
  digitalWrite(Mon_pin, LOW);
  digitalWrite(Mdir_pin, HIGH);

  Wire.begin();
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  server.begin();
}

void loop() {
   boolean drum, DDo, DDc, TPr, TPl;
   

  //get NFC status by i2c once per second
  curr_time = millis();
  delta_time = curr_time - prev_time;
  if (delta_time > 1000) {
    for (byte i = 0; i < 3; i++) {
      NFC_UID[i] = "";
      Wire.beginTransmission(16);
      Wire.write(i);
      Wire.endTransmission();
      delay(3);
      Wire.requestFrom(16, 4);
      while (Wire.available()) {
        byte c = Wire.read();
        NFC_UID[i] += String(c,HEX);
      }
      delay(3);
    }
    prev_time = curr_time;
  }

  //Read PIR status
  drum = digitalRead(PIR5_pin);
  if (drum) {
    PIR5_status = "Move";
  } else {
    PIR5_status = "Clear";
  }

  //Read touchpad status
  drum = digitalRead(TOUCHr_pin);
  if (drum) {
    TOUCHr_status = "Available";
    digitalWrite(TPLedr_pin, LOW);
  } else {
    TOUCHr_status = "Empty";
    digitalWrite(TPLedr_pin, HIGH);
  }
  drum = digitalRead(TOUCHl_pin);
  if (drum) {
    TOUCHl_status = "Available";
    digitalWrite(TPLedl_pin, LOW);
  } else {
    TOUCHl_status = "Empty";
    digitalWrite(TPLedl_pin, HIGH);
  }

  //Read Grate status;
   DDc = digitalRead(DDc_pin);
   DDo = digitalRead(DDo_pin);
  if (DDo&&!DDc) {
    Grate_status = "Closed";
  } else {
    Grate_status = "Open";
  }

  //Reply to network requests
  EthernetClient client = server.available();
  if (client) {
    if (client.available()) {
        switch (client.read()) {
          case 's': //reply with status Grate, PIR5, TOUCHr, TOUCHl, NFC`s;
            client.println("");
            client.print("Grate=");
            client.println(Grate_status);
            client.print("PIR5=");
            client.println(PIR5_status);
            client.print("TOUCHr=");
            client.println(TOUCHr_status);
            client.print("TOUCHl=");
            client.println(TOUCHl_status);
            for (byte i = 0; i < 3; i++) {
              client.print(NFC_REP[i]);
              client.println(NFC_UID[i]);
            }
            break;
          case 'q':               //unlock DLL
            digitalWrite(DLL_pin, HIGH);
            break;
          case 'w':               //lock DLL
            digitalWrite(DLL_pin, LOW);
            break;
          case 'e':               //unlock DLS
            digitalWrite(DLS_pin, HIGH);
            break;
          case 'r':               //lock DLS
            digitalWrite(DLS_pin, LOW);
            break;
          case 'f':               //unlock DLGH
            digitalWrite(DLGH_pin, HIGH);
            break;
          case 'g':               //lock DLGH
            digitalWrite(DLGH_pin, LOW);
            break;
          case 'h':               //unlock DLFAC
            digitalWrite(DLFAC_pin, HIGH);
            break;
          case 'j':               //lock DLFAC
            digitalWrite(DLFAC_pin, LOW);
            break;
          case 'a':               //Open Grate
            ACT = 1;
            break;
          case 'd':               //Close Grate
            ACT = 2;
            break;
        }
     }
  }
  delay(5);
  client.stop();
  door_gate_control();
 
}


void door_gate_control(void)
{
  static int i=0;

 if (ACT == 1) {
  digitalWrite(DLGH_pin, HIGH);
  
                  if (digitalRead(DDo_pin)) {
                                              digitalWrite(Mdir_pin, LOW);
                                              delay(100);
                                              
                                              if(i< 16){                        
                                                         brightness += fadeAmount;
                                                         analogWrite(Mon_pin, brightness);
                                                         delay(WaitDel2);
                                                         i++;
                                                        }
                                              delay(100);
                                   if(i >=16 && i < 16+20) {
                      
                                                     brightness -= fadeAmount;
                                                     analogWrite(Mon_pin, brightness);
                                                     delay(WaitDel3);
                                                     i++;
                                                    }
                      
                                   if(!digitalRead(DDo_pin)){
                                                              digitalWrite(Mon_pin, LOW);}
                                                            }else
    {
    ACT = 0;
    brightness=BRIGHTNESS;
    i=0;
    digitalWrite(Mon_pin, LOW);}
  }
  if (ACT == 2) {
    if (digitalRead(DDc_pin)) {
                                digitalWrite(Mdir_pin, HIGH);
                                delay(100);
                                
                      if(i<16)      {
                                      brightness += fadeAmount;
                                      analogWrite(Mon_pin, brightness);
                                      delay(WaitDel2);
                                      i++;
                                    }
      delay(100);
      if(i>=16 && i <16+18)         {
                                      brightness -= fadeAmount;
                                      analogWrite(Mon_pin, brightness);
                                      delay(WaitDel3);
                                      i++;
                                    }
                                    
 
      if(!digitalRead(DDc_pin)){
      digitalWrite(Mon_pin, LOW);}
    } else {
    ACT = 0;
    brightness=BRIGHTNESS;
    i=0;
    digitalWrite(Mon_pin, LOW);
    }
  }
}
