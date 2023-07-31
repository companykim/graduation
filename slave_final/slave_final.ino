/****LCD I2C setting****/
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

/***I2C setting***/
#include <Wire.h>
#define Slave 0x04
byte rec[128];

#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A8, A9, A10, A11}; //R1 R2 R3 R4
byte colPins[COLS] = {A12, A13, A14, A15}; // C1 C2 C3 C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String input = "";
String PW = "A0203";
int keyCount;

/***MG996R setting***/
#include <Servo.h>
Servo servo;

/****RFID setting****/
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 49
#define SS_PIN 53
MFRC522 mfrc(SS_PIN, RST_PIN);

int val = 0;
int val2 = 0;
int val3 = 0;

int z = 0;

void setup() {  
  /***LCD I2C setup***/
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ready...");
  
  keyCount = 0;
  Wire.begin(Slave);
  SPI.begin();
  Serial.begin(9600);
  servo.attach(3);
  servo.write(160);
  pinMode(48, INPUT);

  mfrc.PCD_Init();
  Serial.println();
}

void loop() 
{ 
  /**** keypad starting ****/
  randomSeed(analogRead(1));
  char key = keypad.getKey();
  
   if (key == 'A') {
    z = 1;
    delay(300);
   }
  
  val = digitalRead(48);
  
  /********* wifi part ***********/
  Wire.onReceive(record);
    
  if (rec[0] == 1) {
    servo.write(0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("open the door");
    delay(5000);
  }
  
  if (rec[0] == 2) {
    if (val == HIGH) {
      servo.write(0);
      lcd.setCursor(0, 1);
      lcd.print("movement..");
      delay(3000);
    }
    
    else {
      rec[0] = 3;
      servo.write(160);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("close the door");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ready...");
     }
    }
  
  /*********** RFID part *********/
  if (z == 0) {
    if ( !mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial() ) {
      return;
    }
    String content = "";
    byte letter;
    for(byte i = 0; i < mfrc.uid.size; i++) {
      content.concat(String(mfrc.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc.uid.uidByte[i], HEX));
    }
    Serial.println();
  
    content.toUpperCase();

    if(content.substring(1) == "63 D8 99 06")
    {
      
      Wire.beginTransmission(Slave);
      Wire.write(3);
      Wire.endTransmission();

      servo.write(0);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("open the door");
      delay(5000);
    
      val2 = digitalRead(48);
      for (int i = 0; i < 50; i++) 
      {
        if (val2 == HIGH) 
        {
          lcd.setCursor(0, 1);
          lcd.print("movement..");
          servo.write(0);
          delay(100);
        }
      }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("close the door");
        servo.write(160);
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ready...");
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("wrong card");
    }
  }
  
   /*** keypad part ***/   
   if (z == 1)
   {
      if (key) 
      {
    
        if (key == '*') 
        {
          if (input == PW)
          {
            Wire.beginTransmission(Slave);
            Wire.write(4);
            Wire.endTransmission();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("open the door");
            input = "";
            keyCount = 0;
            PW = " ";
            servo.write(0);
            delay(5000);
          
            val3 = digitalRead(48);
            for (int i = 0; i < 50; i++)
            {
              if (val3 == HIGH) 
              {
                lcd.setCursor(0, 1);
                lcd.print("movement..");
                servo.write(0); 
                delay(100);
              }
            }
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("close the door");
            servo.write(160);
            delay(3000);
            lcd.clear();
            lcd.print("ready...");
            z = 0;
          }
          else // wrong password
          {
            PW = " ";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("wrong password");
            input = "";
            keyCount = 0;
            
          }
        }

        else if (key == '#') 
        {
          int val = random(0, 9999);
          String ranpw = String(val);
          PW = 'A'+ranpw;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("new password: ");
          lcd.setCursor(0, 1);
          lcd.print(ranpw);
        }
        else 
        {
          input += key;
        }
        delay(100);
      }
   }
}
  
void record(int receiveNum) {
  for (int i = 0; i < receiveNum; i++) {
    rec[i] = Wire.read();
  }
}
