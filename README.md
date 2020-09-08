# 自动同步时间的可掉电保存时间的闹钟
## 简介

这是一个使用NodeMCU-01（ESP-12E）、DS3231实时时钟模块和OLED显示器（SSD1306 128×64）制作的时钟（默认为北京时间)。

This is a progamme for NodeMCU which uses DS3231 model and OLED13864_IIC to show current time(default is Beijing Time) and can also be an alarm.

显示屏使用U8g2库,实时时钟使用RTClib库。

The programme uses u8g2 Library for OLED12864_IIC Display as well as RTClib for DS3231.

u8g2:   https://github.com/olikraus/u8g2

RTClib: https://github.com/adafruit/RTClib
  
## 接线说明/Connecting instructions
SCL--D1(GPIO 5)

SDA--D2(GPIO 4)
  
## 程序特色/Features
  使用NTP时间库与互联网同步时间，默认每30秒同步一次。
  
  The programme uses NTPClient to synchronize the time, synchronize per 30s default.
  
  NTPClient:https://github.com/arduino-libraries/NTPClient
	 
## 备注
  	1.记得把下面的SSID和Password换成你自己Wifi的名称和密码
  	Remember to Change the "ssid" and "passwords" below to the SSID and Password of your own WIFI.
	
  	2.目前存在漏洞，当AP模式开启时只能在连上ESP8266的WIFI后才能使用http://espalarm.local 访问设置页面，否则只能使用ip地址访问。
  	There is a loophole, when the AP mode is turned on, you can use http://espalarm.local access to the settings page only after you connected the WIFI of ESP8266 ,otherwise it can only be accessed using the ip address.
  
  
  #### 小轩010编写/Compile by xiaoxuan010
  ##### 最后更改/Last changes:2020.9.7
