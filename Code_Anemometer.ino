#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include<Wire.h>
#include <RTClib.h>
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27,16,2);
int sensor = 3;
int led = 4;
int count =0;
int RPM = 0;
double windsnelheid = 0.0;
unsigned long previousMillis =0;
const long interval = 1000;
String filename;
File sdcard_file;
int i =1;
int timeSinceStart,difference;
int SamplesArray [5] = {0,0,0,0,0};
void setup() {
 // put your setup code here, to run once:
Serial.begin(9600);
pinMode(sensor,INPUT_PULLUP);
pinMode(CS_pin,OUTPUT);
pinMode(led,OUTPUT);
attachInterrupt(digitalPinToInterrupt(sensor), updateCount, RISING); // pulsen
// LCD SETUP
----------------------------------------------------------------------------------
-------
lcd.init();
lcd.backlight();
// RTC
SETUP-----------------------------------------------------------------------------
--------------
#ifndef ESP8266
 while (!Serial); // wait for serial port to connect. Needed for native USB
#endif
 if (! rtc.begin()) {
 Serial.println("Couldn't find RTC");
 Serial.flush();
 while (1) delay(10);
 }
 if (! rtc.isrunning()) {
 Serial.println("RTC is NOT running, let's set the time!");
 rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
// SD CARD SETUP
----------------------------------------------------------------------------------
------
SD.begin();
DateTime now = rtc.now();
filename = String(now.day()) + String(now.hour()) + String(now.minute()) +
String(now.second()) + ".csv";
sdcard_file = SD.open(filename, FILE_WRITE);
if(sdcard_file)
{
 sdcard_file.print("TIMESTAMP");
 sdcard_file.print(", ");
 sdcard_file.print("TIME_SINCE_START_MEASUREMENT");
 sdcard_file.print(", ");
 sdcard_file.print("WINDSPEED");
 sdcard_file.print(", ");
 sdcard_file.print("RPM");
 sdcard_file.close();
}
else
{
 Serial.println("error opening the file");
}
// SERIAL HEADER SETUP
----------------------------------------------------------------------------------
----
Serial.print("TIMESTAMP");
Serial.print(", ");
Serial.print("TIME_SINCE_START_MEASUREMENT");
Serial.print(", ");
Serial.print("WINDSPEED");
Serial.print(", ");
Serial.print("RPM");
}
void loop() {
 // put your main code here, to run repeatedly:
unsigned long currentMillis = millis ();
if (currentMillis - previousMillis >= interval)
{
 previousMillis = currentMillis;
 MovingAverage(SamplesArray, count);
 RPM = CalculateRPM(SamplesArray);
 windsnelheid = CalculateVelocity(RPM);
 PrintToSDCard( millis(), RPM,windsnelheid);
 count = 0;
}
digitalWrite(led,HIGH);
PrintToLCD();
}
int CalculateRPM( int arr[5]){
 int sum =0;
 int RPM = 0;
 for (int i=0; i<5;i++){
 sum+=arr[i];
 }
 double average = double(sum) /5;
 RPM = int ((average/12)*60);
 return RPM;
}
void MovingAverage(int arr [], int value)
{
 for (int i =4; i>=1; i--){
 arr[i] = arr[i-1];
 }
 arr[0] = value;
}
double CalculateVelocity(int value)
{
 double v = 0;
 if (value !=0){ v = 0.0325*value + 1.5235;}
 return v;
}
void PrintToSDCard( double runningtime, int value , double v)
{
 DateTime now = rtc.now();
 sdcard_file = SD.open(filename, FILE_WRITE);
 if(sdcard_file)
 {
 sdcard_file.println();
 sdcard_file.print(now.year(),DEC);
 sdcard_file.print(now.month(),DEC);
 sdcard_file.print(now.day(),DEC);
 sdcard_file.print(now.hour(),DEC);
 sdcard_file.print(now.minute(),DEC);
 sdcard_file.print(now.second(),DEC);
 sdcard_file.print(", ");
 sdcard_file.print(runningtime);
 sdcard_file.print(", ");
 sdcard_file.print(v);
 sdcard_file.print(", ");
 sdcard_file.print(value);
 sdcard_file.close();
 }
}
void PrintToLCD()
{
 lcd.setCursor(0, 0);
 lcd.print("RPM: ");
 lcd.setCursor(5, 0);
 lcd.print(RPM);
 lcd.setCursor(0, 1);
 lcd.print("Snelheid: ");
 lcd.setCursor(11, 1);
 lcd.print(windsnelheid);
 lcd.clear();
}
void updateCount()
{
 count++;
}
