#include <SPI.h>
#include <SD.h>
//#include <Wire.h> //library untuk wire i2c
#include <RFID.h> //library RFID
#include <Ethernet.h>
#include <DS3231.h>

File myFile;

DS3231  rtc(SDA, SCL);




byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

// the dns server ip
//IPAddress dnServer(192, 168, 1, 1);
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 255, 0);

//Variable IP Ethernet
String ip_txt;
String gateway_txt;
String subnet_txt;
String server_txt;

EthernetServer server(80);


#define sda 3 //Pin Serialdata (SDA)
#define rst 8 //pin Reset 

#define buzzer 6 //buzzer
#define mode 19 //switch mode, active low
int Mode;

RFID rfid(sda,rst);

String id_card="";
String save_rfid="";
String PostHeader;
int port;
unsigned long now;
EthernetClient client;

//library Time
int tgl,bln,thn,jam,mnt,dtk;

Time saveTime;
int jamNow;
int jamTemp=0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  pinMode(mode, INPUT); //active low
  digitalWrite(mode, HIGH);
  
  rtc.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  //-----------------------------------------------------------------Open file IP, Subnet, Gateway, Server ---------------
  //---------------------------------------------------IP.TXT
  myFile = SD.open("IP.TXT"); //membuka file di sd card
  if (myFile) {
    Serial.println("IP.TXT :");
    while (myFile.available()) {
      ip_txt += myFile.readString(); //menyimpan ip di variable ip_txt
    }
    Serial.println(ip_txt);
    myFile.close();
  }else{
    Serial.println("error opening test.txt");
  }
  Serial.println();
  
//------------------------------------------------------Gateway.txt
  myFile = SD.open("gateway.txt"); //membuka file di sd card
  if (myFile) {
    Serial.println("GATEWAY.TXT :");

    while (myFile.available()) {
      gateway_txt += myFile.readString(); //menyimpan ip di variable ip_txt
    }
    Serial.println(gateway_txt);
    myFile.close();
  }else{
    Serial.println("error opening test.txt");
  }
  Serial.println();
//------------------------------------------------------Subnet.txt
  myFile = SD.open("subnet.txt"); //membuka file di sd card
  if (myFile) {
    Serial.println("SUBNET.TXT :");

    while (myFile.available()) {
      String x = myFile.readString();
      subnet_txt += x; //menyimpan ip di variable ip_txt
    }
    Serial.println(subnet_txt);
    myFile.close();
  }else{
    Serial.println("error opening test.txt");
  }
  Serial.println();
  
//------------------------------------------------------SERVER.txt
  myFile = SD.open("server.txt"); //membuka file di sd card
  if (myFile) {
    Serial.println("SERVER.TXT :");

    while (myFile.available()) {
      String x = myFile.readString();
      server_txt += x; //menyimpan ip di variable ip_txt
    }
    Serial.println(server_txt);
    myFile.close();
  }else{
    Serial.println("error opening test.txt");
  }
  Serial.println();
  
 //---------------------------------------------------------------------------Save to IP----
  int Parts2[4] = {0,0,0,0};
  int Part2 = 0;
  for ( int i=0; i<ip_txt.length(); i++ ){
    char c = ip_txt[i];
    if ( c == '.' ){
      Part2++;
      continue;
    }
    Parts2[Part2] *= 10;
    Parts2[Part2] += c - '0';
  }
  IPAddress ip( Parts2[0], Parts2[1], Parts2[2], Parts2[3] );
  
 //---------------------------------------------------------------------------Save to GATEWAY----
  int Parts3[4] = {0,0,0,0};
  int Part3 = 0;
  for ( int i=0; i<gateway_txt.length(); i++ ){
    char c = gateway_txt[i];
    if ( c == '.' ){
      Part3++;
      continue;
    }
    Parts3[Part3] *= 10;
    Parts3[Part3] += c - '0';
  }
  IPAddress gateway( Parts3[0], Parts3[1], Parts3[2], Parts3[3] );
  
 //---------------------------------------------------------------------------Save to SUBNET----
  int Parts4[4] = {0,0,0,0};
  int Part4 = 0;
  for ( int i=0; i<subnet_txt.length(); i++ ){
    char c = subnet_txt[i];
    if ( c == '.' ){
      Part4++;
      continue;
    }
    Parts4[Part4] *= 10;
    Parts4[Part4] += c - '0';
  }
  IPAddress subnet( Parts4[0], Parts4[1], Parts4[2], Parts4[3] );
  //-----------------*/
  Ethernet.begin(mac, ip, gateway, subnet);
  //print out the IP address
  Serial.print("IP \t\t\t= ");
  Serial.println(Ethernet.localIP());
  Serial.print("Gateway \t\t= ");
  Serial.println(gateway);
  Serial.print("Subnet \t\t\t= ");
  Serial.println(subnet);
  Serial.println("RFID");

  SPI.begin(); //Prosedur antarmuka SPI
  rfid.init(); //Memulai inialisasi module RFID
  port=12080;
  Serial.println("Dekatkan Kartu ID");
}

void syncTime(){
  bool use = false;
  String reply = "";
  
  if (client.connect(server_txt.c_str(), port)) {
    Serial.println("connected");
    client.println("GET /attendance/get_time HTTP/1.1");
//    client.println("Connection: keep-alive");
    client.println("Host: " + server_txt + ":" + String(port));
    client.println("Connection: close");
    client.println();
    delay(100);
        
    String reply_server;
    if (client.available()) {
      String xx = String(client.readString());
      for ( int i=0; i < xx.length(); i++ ){
        if (xx[i] == '{') {
          use = true;
          continue;
        }  
        if (xx[i] == '}') {
          continue;
        }
        if (use) {
          reply += xx[i];
        }
      }
    }
    Serial.println(reply);
    Serial.println("lanjot");
    int dateTimes[6] = {0,0,0,0,0,0};
    int dateTime = 0;
    for ( int t=0; t<reply.length(); t++ ){
      char c = reply[t];
      if ( c == '.' ){
        dateTime++;
        continue;
      }
      dateTimes[dateTime] *= 10;
      dateTimes[dateTime] += c - '0';
    }
    tgl=dateTimes[0];
    bln=dateTimes[1];
    thn=dateTimes[2];
    jam=dateTimes[3];
    mnt=dateTimes[4];
    dtk=dateTimes[5];
    
    rtc.setTime(jam, mnt, dtk);
    rtc.setDate(tgl, bln, thn);
    
    Serial.print("Date = ");
    Serial.print(tgl);
    Serial.print(bln);
    Serial.println(thn);
    Serial.print("Jam = ");
    Serial.print(jam);
    Serial.print(mnt);
    Serial.println(dtk);
    Serial.println();
    //client.stop();
    Serial.println("Sync Time DONE.!!!");
  }
  else{
    Serial.println("Connection Failed..!!!");
  }
}

void tapCard(){
  id_card = "";
  bool use = false;
  String reply = "";
  if(rfid.isCard()){
    if(rfid.readCardSerial()){
      id_card += rfid.serNum[0];
      id_card += rfid.serNum[1];
      id_card += rfid.serNum[2];
      id_card += rfid.serNum[3];
      id_card += rfid.serNum[4];

      // IF CLIENT OK 
      Serial.println(server_txt);
      Serial.println("Tap Card");
      
      if (client.connect(server_txt.c_str(), port)) {
        
        Serial.println("connected");
        // request to server
        
        String timeTap=rtc.getTimeStr();
        String dateTap=rtc.getDateStr();
        client.println("GET /attendance_school/make?rfid=" + id_card + "&timestamp=" + dateTap + "T" + timeTap + " HTTP/1.1");
        Serial.println(id_card);
        Serial.println(timeTap);
        Serial.println(dateTap);
        
        //client.println("Connection: keep-alive");
        client.println("Host: " + server_txt + ":" + String(port));
        client.println("Connection: close");
        client.println();
        delay(100);
//////////////////////////////////////////////////////////////////PRINT REPLAY

        Serial.println(client.available());
        String reply_server;
        if (client.available()) {
          String xx = String(client.readString());
          for ( int i=0; i < xx.length(); i++ ){
            if (xx[i] == '{') {
              use = true;
            }  
            if (use) {
              reply += xx[i];
            }
          }
        }
        
        Serial.println(reply);
        Serial.println();
        //client.stop();
        if(reply=="{1}"){
          Serial.println("Data Found");
          speaker(1);
        }else{
          Serial.println("Data Not Found");
          speaker(2);
        }
        Serial.println("DONE");
      }
      else{
        Serial.println("Connction Failed");
        Serial.println("Save to Card.........");
        speaker(2);
      }
       
      // END READ
      delay(100);   
    }
  } 
}

void tapRegistered(){
  id_card = "";
  bool use = false;
  String reply = "";
  if(rfid.isCard()){
    if(rfid.readCardSerial()){
      id_card += rfid.serNum[0];
      id_card += rfid.serNum[1];
      id_card += rfid.serNum[2];
      id_card += rfid.serNum[3];
      id_card += rfid.serNum[4];

      // IF CLIENT OK 
      Serial.println(server_txt);
      Serial.println("Tap Registered");
      
      if (client.connect(server_txt.c_str(), port)) {
        
        Serial.println("connected");
        // request to server
        
        String timeTap=rtc.getTimeStr();
        String dateTap=rtc.getDateStr();
        client.println("GET /attendance_school/register_card?rfid=" + id_card  + " HTTP/1.1");
        Serial.println(id_card);
        Serial.println(timeTap);
        Serial.println(dateTap);
        
        //client.println("Connection: keep-alive");
        client.println("Host: " + server_txt + ":" + String(port));
        client.println("Connection: close");
        client.println();
        delay(100);
        Serial.println("DONE");
      }
      else{
      }
       
      // END READ
      delay(100);   
    }
  } 
}

void speaker(int n){
  if(n==1){//success
    tone(buzzer, 1500);
    delay(100);
    noTone(buzzer);
    delay(50);
    tone(buzzer, 1500);
    delay(100);
    noTone(buzzer);
  }
  if(n==2){//failed
    tone(buzzer, 300);
    delay(300);
    noTone(buzzer);
    delay(50);
    tone(buzzer, 500);
    delay(700);
    noTone(buzzer);
  }
  if(n==3){
    
  }
}

void loop() {
  Mode=digitalRead(mode);

  saveTime=rtc.getTime(); //untuk memyimpan waktu
  jamNow = saveTime.min;
  
  if((jamTemp + 1) == jamNow){
    Serial.print("Sync Time ");
    syncTime();
//    delay(50);
    jamTemp=jamNow;
  }
  else if(Mode==LOW){
    tapRegistered();
  }
  else{
    tapCard();
    jamTemp=jamNow;
  }
}
