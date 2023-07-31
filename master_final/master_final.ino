/****I2C setting****/
#include <Wire.h>
#define Slave 0x04
byte rec[128];

/****ESP-01 setting****/
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //RX TX
#define DEBUG true

/***SD setting***/
#include <SD.h>
#include <SPI.h>

/***DS3231 setting***/
#include <DS3231.h>
DS3231 clock;
RTCDateTime dt1;
RTCDateTime dt2;
RTCDateTime dt3;

void setup() {
  
  Wire.begin(Slave);
  SPI.begin();
  Serial.begin(9600);
  clock.begin();
  mySerial.begin(9600);

/****SD setup****/
  Serial.println("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("Card failed");
    while(1);
  }
  Serial.println("Card initialized");
  Serial.println();

  /***** wifi connecting *****/
  sendData("AT+RST\r\n",2000,DEBUG); 
  sendData("AT+CWMODE=1\r\n",1000,DEBUG); 
  sendData("AT+CWJAP=\"SK_WiFiGIGA5D9C\",\"1809064185\"\r\n", 5000, DEBUG); 
  delay(1000);
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); 
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); 

  Serial.println("connected");

/****DS3231 setup****/
  clock.setDateTime(__DATE__, __TIME__);
}

void loop() 
{
  /****ESP-01 loop***/
  int pinNumber;
  if(mySerial.available()) //
  {
    if(mySerial.find("+IPD,")) 
    {
      delay(1000); 
      int connectionID = (mySerial.read()-48);      
      mySerial.find("pin=");

      pinNumber = (mySerial.read()-48)*10;
      pinNumber += (mySerial.read()-48);
      
      if (pinNumber == 13)
      {
        /******SD(wifi)******/
        File dataFile1 = SD.open("DB.txt", FILE_WRITE);
        if (dataFile1) {
          dt1 = clock.getDateTime();
          dataFile1.print(dt1.year); dataFile1.print("-");
          dataFile1.print(dt1.month); dataFile1.print("-");
          dataFile1.print(dt1.day); dataFile1.print(" ");
          dataFile1.print(dt1.hour); dataFile1.print(":");
          dataFile1.print(dt1.minute); dataFile1.print(":");
          dataFile1.print(dt1.second);
          dataFile1.println(" door used by smartphone");
          dataFile1.close();
        }
        /****open at wifi****/
        Wire.beginTransmission(Slave);
        Wire.write(1);
        Wire.endTransmission();
        delay(5000);

        /*****close at wifi****/
        Wire.beginTransmission(Slave);
        Wire.write(2);
        Wire.endTransmission();
        delay(1000); 
    
        String closeCommand = "AT+CIPCLOSE=";
        closeCommand += connectionID;
        closeCommand += "\r\n";
        sendData(closeCommand, 1000, DEBUG);
      }
    }
  } 
  
  /**RFID loop***/
  Wire.onReceive(record);
  if (rec[0] == 3) {
    /*****SD(RFID)*****/
    File dataFile2 = SD.open("DB.txt", FILE_WRITE);
    if (dataFile2) {
      dt2 = clock.getDateTime();
      dataFile2.print(dt2.year); dataFile2.print("-");
      dataFile2.print(dt2.month); dataFile2.print("-");
      dataFile2.print(dt2.day); dataFile2.print(" ");
      dataFile2.print(dt2.hour); dataFile2.print(":");
      dataFile2.print(dt2.minute); dataFile2.print(":");
      dataFile2.print(dt2.second);
      dataFile2.println(" door used by RFID");
      dataFile2.close();
    }
    rec[0] = 10;
  }
  
  if (rec[0] == 4) {
    /******SD(Keypad)*****/
    File dataFile3 = SD.open("DB.txt", FILE_WRITE);
    if (dataFile3) {
      dt3 = clock.getDateTime();
      dataFile3.print(dt3.year); dataFile3.print("-");
      dataFile3.print(dt3.month); dataFile3.print("-");
      dataFile3.print(dt3.day); dataFile3.print(" ");
      dataFile3.print(dt3.hour); dataFile3.print(":");
      dataFile3.print(dt3.minute); dataFile3.print(":");
      dataFile3.print(dt3.second);
      dataFile3.println(" door used by keypad");
      dataFile3.close();
    }
    rec[0] = 20;
  }
}
/***ESP-01의 정보를 설정하기 위한 함수**/
String sendData(String command, const int timeout, boolean debug) {
    String response = "";           
    mySerial.print(command);
    long int time = millis();
    while( (time+timeout) > millis()) {
      while(mySerial.available()) {
        char c = mySerial.read();
        response+=c;
      }  
    }    
    if(debug) {
      Serial.print(response);
    }  
    return response;
}
/***I2C 통신을 위한 함수***/
void record(int receiveNum) {
  for (int i = 0; i < receiveNum; i++) {
    rec[i] = Wire.read();
  }
}
