
/*
connection pins
SCL=22;
SDA=21
PIR=36
BUZZER=39
GAS_SENSOR=34
RELAY FOR FAN=35
SOIL_MOISTURE_SENSOR=32
RELAY FOR PUMP =33
*/

#define BLYNK_TEMPLATE_ID "TMPL2xiB5SU6H"
#define BLYNK_TEMPLATE_NAME "Smart Farm monitoring system"
#define BLYNK_AUTH_TOKEN "YWfsHUXA4aCrHh9U14rO4-SjyBcDViMK"

//include libraries
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

const int PIR_PIN=36;
const int BUZZER_PIN=32;
const int GAS_SENSOR=34;//Analog pin
const int RELAY_FAN =0;
const int SOIL_MOISTURE_PIN=35;//Analog pin
const int RELAY_PUMP =4;
const int ECHO_PIN=14;
const int TRIGGER_PIN=26;

const int gasThreshhold=100;
const int buzzerState=digitalRead(BUZZER_PIN);
const int soil_Threshold=395;//normally 399
const int tankHeight=100;
float level;
const int minLevel=20;
const int maxLevel=80;
const int irrigationDuration=30000;
bool irrigationActive = false;
int relay_State=LOW;



//Initialise the LCD Screen
LiquidCrystal_I2C lcd(0x3F, 16, 2);



char auth[] = BLYNK_AUTH_TOKEN;  //Enter your Blynk Auth token
char ssid[] = "SAPA_NET 1.1";  //Enter your WIFI SSID
char pass[] = "resilience";  //Enter your WIFI Password

DHT dht(15,DHT11);//PLACE PIN NUMBER, DHT11);//(DHT sensor pin,sensor type)  D4 DHT11 Temperature Sensor
BlynkTimer timer;


//DHT function
void DHT (){
    float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to avoid NaN)
  if (isnan(h) || isnan(t)) {
    Serial.print("Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.println("°C");
  Blynk.virtualWrite(V0,h);
  Blynk.virtualWrite(V1,t);

   delay(2000);// Read sensor data every 2 seconds

}


//intrusion function
 void intrusion (){
const int pirState=digitalRead(PIR_PIN);
  if (pirState==HIGH && buzzerState!=HIGH){
   Serial.println("Intrusion Detected!");
  digitalWrite(BUZZER_PIN,HIGH);//you can replace with function tone(BUZZER_PIN,1000)
 Blynk.logEvent("PIR MOTION","Warning! Motion Detected");
  //Blynk.email("chimeziejoshuac124@gmail.com","PIR MOTION","Warning! Motion Detected");
  //Blynk.sms("+2347012062584","Warning! Motion Detected in the Farm");
 }
 else{
    digitalWrite(BUZZER_PIN,LOW);
    buzzerState==LOW;
  }
  delay(1500);
  
}


//Gas Leakage Function
void gasLeakage(){
  const int gas_Level=analogRead(GAS_SENSOR);
  const int fanRelay_State=digitalRead(RELAY_FAN);
  if(gas_Level > gasThreshhold && fanRelay_State!=HIGH ){
    Serial.print("Gas level:");
    Serial.print(gas_Level);
    Serial.print("Gas leakage Detected");
    digitalWrite(RELAY_FAN,HIGH);
    Blynk.logEvent("Gas Leakage","ATTENTION! Gas leakage Detected");
    //Blynk.email("chimeziejoshuac124@gmail.com","Gas Leakage","ATTENTION! Gas leakage Detected");
    //Blynk.sms("+2347012062584","ATTENTION!: Gas Leakage Detected");
    //if (buzzerState!=HIGH){
     // tone(BUZZER_PIN,1000);
    //}
  }
}

void soilMoisture(){
  int soilMoisture=analogRead(SOIL_MOISTURE_PIN);
  soilMoisture=map(soilMoisture,0,1024,0,100);
  const int soilMoisture_level=(soilMoisture-100) * -1;
Serial.println("soilMoisture");
Serial.println(soilMoisture);

  Blynk.virtualWrite(V3,soilMoisture);
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(soilMoisture_level);
  lcd.print(" ");

  if(soil_Threshold > soilMoisture /*&& level>minLevel*/ ){
    Serial.println("Irrigation System Activated");
    digitalWrite(RELAY_PUMP,HIGH);
    Serial.println("Water Pump Turned ON");
    irrigationActive=true;
    
  }
else {
digitalWrite(RELAY_PUMP,LOW);
irrigationActive=false;
Serial.println("Water Pump OFF");

}
}


void waterLevel(){
long duration,distance;
digitalWrite(TRIGGER_PIN,LOW);
delayMicroseconds(2);
digitalWrite(TRIGGER_PIN,HIGH);
delayMicroseconds(10);
digitalWrite(TRIGGER_PIN,LOW);
duration = pulseIn(ECHO_PIN,HIGH);
distance=(duration/2)/29.1;
level=tankHeight-distance;
Serial.print("Tank Level:");
Serial.print(level);
Blynk.virtualWrite(V2,level);
}



void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);//baud rate

//lcd setup
lcd.begin();
lcd.backlight();
//bool result = Blynk.connect();
//Serial.print(result);
//blynk initialise
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);




dht.begin();//initialise DHT sensor


//pinMode definition
pinMode(PIR_PIN,INPUT);
pinMode(BUZZER_PIN,OUTPUT);
pinMode(GAS_SENSOR,INPUT);
pinMode(RELAY_FAN,OUTPUT);
pinMode(SOIL_MOISTURE_PIN,INPUT);
pinMode(RELAY_PUMP,OUTPUT);
pinMode(TRIGGER_PIN,OUTPUT);
pinMode(ECHO_PIN,INPUT);    
pinMode(2,OUTPUT);

//intrusion();

}

BLYNK_WRITE(VPIN_BUTTON_1) {
 relay_State= param.asInt();
  digitalWrite(RELAY_PUMP, relay_State);
}

void loop() {
  // put your main code here, to run repeatedly: 
DHT();
intrusion();
gasLeakage();
soilMoisture();
waterLevel();
Blynk.run();
timer.run();

}
