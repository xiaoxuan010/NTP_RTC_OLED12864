#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "RTClib.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include "index_CN.h"

#define cirR 25             //Radius of the dial
//EEPROM Address Settings
#define dispRotationAddr 0  
#define alarmHourAddr    1
#define alarmMinuteAddr  2
#define isAlarmOnAddr    3

#define turnScreen_ON  0x01
#define turnScreen_OFF 0x00
#define alarm_ON       0x01
#define alarm_OFF      0x00

typedef struct{int x;int y;}pointPos;

const char *host     = "espalarm";  //you can visit http://espalarm.local in your broswer 
                                    //to change some settings for the alarm
const char *ssid     = "1108";
const char *password = "13600066680";
const char *asAPssid = "ESP8266alarm";
const char *asAPpass = "xiaoxuan010";
const char *daysOfWeek[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const pointPos CCpos = {25,25};
const float sin30n[] = {0.0000000 ,0.5000000 ,0.8660254 ,1.0000000 ,0.8660254 ,0.5000000 ,0.0000000 ,-0.5000000 ,-0.8660254 ,-1.0000000 ,-0.8660254 ,-0.5000000 };
const float cos30n[] = {1.0000000 ,0.8660254 ,0.5000000 ,0.0000000 ,-0.5000000,-0.8660254,-1.0000000,-0.8660254 ,-0.5000000 ,0.0000000  ,0.5000000  ,0.8660254  };

WiFiUDP ntpUDP;
RTC_DS3231 rtc;            // ↓↓↓↓↓↓↓↓↓↓ NTP Server. you can use other Servers such as pool.ntp.org
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60*60*8, 30000);
ESP8266WebServer server(80);//Jet lag (in seccond)↑↑↑↑↑↑ ↑↑↑↑↑update Interval (in ms)                
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE);

unsigned long long LastUpdateTime = 0;
unsigned long long LastReportTime = 0;
unsigned long long RTCst;
unsigned long long LastClickTime = 0;
unsigned long long alarmPauseTime = 0;
unsigned long long turnCommitTime = 0;
byte turnScreen = turnScreen_OFF;
byte isAlarmOn = alarm_OFF;
byte alarmHour = 0;
byte alarmMinute = 0;
bool isAlarmPause = false;
bool isSqwOn = false;
bool isNetBegin = false;
bool shouleCommit = false;
bool isMDNSbegin = false;
uint8_t isSavePower = 0;

void report(){
//  Serial.print("report\n");
  
  DateTime now = DateTime(RTCst+millis()/1000);
  if(!isSavePower){
    unsigned long todaySec = now.hour()*3600+now.minute()*60+now.second();
    unsigned int sThet = now.second()*6,
                 mThet = todaySec/10 ,
                 hThet = todaySec/120;
    pointPos sPos{21*sin(radians(sThet)),21*cos(radians(sThet))},
             mPos{17*sin(radians(mThet)),17*cos(radians(mThet))},
             hPos{11*sin(radians(hThet)),11*cos(radians(hThet))};
    
    String hstr = String(now.hour()),
           mstr = now.minute()<10?"0"+String(now.minute()):String(now.minute()),
           sstr = now.second()<10?"0"+String(now.second()):String(now.second()),
           timeStr = hstr+":"+mstr+":"+sstr;
  
    String ystr = now.year()<10?"0"+String(now.year()):String(now.year()),
           Mstr = String(now.month()),
           dstr = String(now.day()),
           dowstr = String(daysOfWeek[now.dayOfTheWeek()]),
           dateStr = ystr+"."+Mstr+"."+dstr+" "+dowstr;
  
    u8g2.firstPage();
    do{
      
      //画白色表盘
      u8g2.setDrawColor(1);
      u8g2.drawDisc(CCpos.x,CCpos.y,cirR);
      //刻度
      u8g2.setDrawColor(0);
      for(byte i = 0;i<12;i++){
        pointPos out = {CCpos.x+ cirR   *sin30n[i],CCpos.y- cirR   *cos30n[i]},
                 in  = {CCpos.x+(cirR-1)*sin30n[i],CCpos.y-(cirR-1)*cos30n[i]};
        u8g2.drawLine(out.x,out.y,in.x,in.y);
      }
      //画出指针
      u8g2.drawDisc(CCpos.x,CCpos.y,2);
      u8g2.drawLine(CCpos.x,CCpos.y,CCpos.x+mPos.x,CCpos.y-mPos.y);
      u8g2.drawLine(CCpos.x,CCpos.y,CCpos.x+hPos.x,CCpos.y-hPos.y);
      u8g2.drawLine(CCpos.x,CCpos.y,CCpos.x+sPos.x,CCpos.y-sPos.y);
      
      //打印时间和日期
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_crox4tb_tn);
      byte timeFontMaxHeight = u8g2.getMaxCharHeight();
      u8g2.drawStr(cirR*2+2+(u8g2.getDisplayWidth()-cirR*2+2)/2-u8g2.getStrWidth(timeStr.c_str())/2,timeFontMaxHeight,timeStr.c_str());
      u8g2.setFont(u8g2_font_t0_11_tf);
      u8g2.drawStr(cirR*2+2+(u8g2.getDisplayWidth()-cirR*2+2)/2-u8g2.getStrWidth(dateStr.c_str())/2,u8g2.getMaxCharHeight()+timeFontMaxHeight,dateStr.c_str());
    }while(u8g2.nextPage());
  }
  
  if(isAlarmOn==alarm_ON){
    if(now.hour()==alarmHour && now.minute()==alarmMinute){
      if(!isSqwOn && !isAlarmPause){
        rtc.writeSqwPinMode(DS3231_SquareWave1Hz);
        isSqwOn=true;
        Serial.println("Alarm On");
      }
    }
  }
  else{
    if(isSqwOn){
      rtc.writeSqwPinMode(DS3231_OFF);
      isSqwOn=false;
    }
  }

  if(millis()-alarmPauseTime>60000 && isAlarmPause)
    isAlarmPause=false;
  
//  Serial.println("end report");
  return;
}
void waitUntilNextSecondRTC(){
//  Serial.println("wunsR");
  unsigned long long lt1 = RTCst + millis()/1000;
  unsigned long long tt1 = RTCst + millis()/1000;
  while(tt1 == lt1){
//    Serial.print("while R");
    if(millis()-LastReportTime>=1000) 
      report();
    else 
      delay(1);
    tt1 = RTCst + millis()/1000;
  }
  return;
}
void waitUntilNextSecondNTP(){
//  Serial.println("wunsN");
  unsigned long long lt2 = timeClient.getEpochTime();
  unsigned long long tt2 = timeClient.getEpochTime();
  while(tt2 == lt2){
//    Serial.print("while N");
    if(millis()-LastReportTime>=1000) 
      report();
    else 
      delay(10);
    tt2 = timeClient.getEpochTime();
  }
  return;
}
void serverBegin(){
  if (!MDNS.begin(host)) 
    Serial.println("Error setting up MDNS responder!"); 
  else
    Serial.println("\nmDNS responder started");
  
  server.on("/", HTTP_GET, [](){
    Serial.println("New connection");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", PAGE_INDEX);
    //Serial.println("Finish connection");
  });

  server.on("/settings/alarm",HTTP_POST,[](){
    server.sendHeader("Connection","close");
    String html = "<h3>上传成功</h3><a href='\\'>返回</a>";
    server.send(200,"text/html; charset=utf-8",html.c_str());
    isAlarmOn = server.arg("isAlarmOn")=="true"?alarm_ON:alarm_OFF;
    EEPROM.write(isAlarmOnAddr,isAlarmOn);
    Serial.printf("IAO:%d\t",isAlarmOn);
    if(isAlarmOn==alarm_ON){
      String WebAlarmTimeStr = server.arg("alarmTime");
      byte cutIndex = WebAlarmTimeStr.indexOf(":");
      alarmHour   = WebAlarmTimeStr.substring(0,cutIndex).toInt();
      alarmMinute = WebAlarmTimeStr.substring(cutIndex+1,WebAlarmTimeStr.length()).toInt();
      EEPROM.write(alarmHourAddr,alarmHour);
      Serial.printf("alaarmHour:%d\n",alarmHour);
      EEPROM.write(alarmMinuteAddr,alarmMinute);
      Serial.printf("am:%d\n",alarmMinute);
      isAlarmPause = false;
    }
    EEPROM.commit();
  });

  server.on("/settings/display",HTTP_POST,[](){
    server.sendHeader("Connection","close");
    String html = "<h3>上传成功</h3><a href='\\'>返回</a>";
    server.send(200,"text/html; charset=utf-8",html.c_str());
    isSavePower = server.arg("isSavePower")=="0"?0:1;
    u8g2.setPowerSave(isSavePower);
  });
  
  server.on("/update", HTTP_POST, [](){
    server.sendHeader("Connection", "close");
    String html = "<h3>上传";
    html += Update.hasError()?"失败":"成功";
    html += "</h3><h4>";
    html += Update.hasError()?"fail":"success";
    html += "</h4><a href='\\'>返回</a>";
    server.send(200,"text/html; charset=utf-8",html.c_str());
    delay(1000);
    ESP.restart();
  },[](){
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if(!Update.begin(maxSketchSpace)){//start with max available size
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_WRITE){
      if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_END){
      if(Update.end(true)){ //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("mDNS Ready, Host:%s\n", host);
}
void myEEPROMbegin(){
  EEPROM.begin(512);
  turnScreen = EEPROM.read(dispRotationAddr);
  if(turnScreen>=3)
    turnScreen = 0;
  Serial.printf("\nturnScreen:%d\t",turnScreen);
  u8g2.setDisplayRotation(turnScreen?U8G2_R2:U8G2_R0);
  
  isAlarmOn = EEPROM.read(isAlarmOnAddr);
  Serial.printf("isAlarmOn=%d\t",isAlarmOn);
  if(isAlarmOn>=3)
    isAlarmOn = 0;
  
  alarmHour = EEPROM.read(alarmHourAddr);
  Serial.printf("\nalarmHour=%d\t",alarmHour);
  if(alarmHour>=24)
    alarmHour = 0;

  alarmMinute = EEPROM.read(alarmMinuteAddr);
  Serial.printf("\nalarmMinute=%d\n",alarmMinute);
  if(alarmMinute>=60)
    alarmMinute = 0;
  return;
}
void buttonHandle(){
  if(millis()-LastClickTime >= 200){
    if(!isAlarmPause && isSqwOn) {
        isAlarmPause = true;
        rtc.writeSqwPinMode(DS3231_OFF);
        isSqwOn = false;
        Serial.println("Alarm Pause");
        alarmPauseTime = millis();
    }
    
    if(isSavePower){
      isSavePower = 0;
      u8g2.setPowerSave(0);
    }
    else{
      if(turnScreen == turnScreen_OFF)
        turnScreen = turnScreen_ON;
      else
        turnScreen = turnScreen_OFF;
      u8g2.setDisplayRotation(turnScreen?U8G2_R2:U8G2_R0);
      EEPROM.write(dispRotationAddr,turnScreen);
      shouleCommit = !shouleCommit;
      report();
    }
    LastClickTime = millis();
  }
}
void setup(){
  Serial.begin(115200);
  Serial.println();
  
  u8g2.begin();
  myEEPROMbegin();
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    rtc.writeSqwPinMode(DS3231_OFF);
    u8g2.firstPage();
    do{
      u8g2.setFont(u8g2_font_crox4tb_tn);
      u8g2.drawStr(0,u8g2.getMaxCharHeight(),"Can't Find RTC!");
    }while(u8g2.nextPage());
    while(true);
  }
  else
    Serial.println("RTC began");
  rtc.writeSqwPinMode(DS3231_OFF);
  RTCst = rtc.now().unixtime()-millis()/1000;
  
  WiFi.mode(WIFI_AP_STA);
//  WiFi.mode(WIFI_STA);
  WiFi.softAP(asAPssid,asAPpass);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  serverBegin();
  pinMode(10,INPUT_PULLUP);
  report();
}
void loop() {
  if(WiFi.status()==WL_CONNECTED && !isNetBegin){
     timeClient.begin();
     server.begin();
     Serial.println("NetBegan");
     isNetBegin = true;
  }
  MDNS.update();
  server.handleClient();
  if(shouleCommit && millis()-LastClickTime>10000){
    EEPROM.commit();
    Serial.println("Commit!");
    shouleCommit = false;
  }
  
  if(digitalRead(10)==LOW)
    buttonHandle();
  
  if(millis()-LastReportTime>=1000){
    LastReportTime = millis();
    report();
    if((LastUpdateTime==0 || millis()-LastUpdateTime>=60000) && WiFi.status()==WL_CONNECTED){
      LastUpdateTime = millis();
      Serial.print("Time Update ");
      if(timeClient.forceUpdate()){
        waitUntilNextSecondNTP();
        rtc.adjust(DateTime(timeClient.getEpochTime()));
        RTCst = timeClient.getEpochTime()-millis()/1000;
        Serial.println("End Update");
      }
      else
        Serial.println("failure");
    }
  }
  delay(isSavePower?50:1);
}
