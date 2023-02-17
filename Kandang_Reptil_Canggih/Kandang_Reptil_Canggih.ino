#include "DHT.h"
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <DS3231.h>

#define humon digitalWrite(11, HIGH)
#define humof digitalWrite(11, LOW)
#define heton digitalWrite(6, HIGH)
#define hetof digitalWrite(6, LOW)
#define lamp analogWrite(10, pwm_lampu)
#define fan analogWrite(5, pwm_fan)
#define temp float(dht.readTemperature())
#define humi int(dht.readHumidity())

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(9, DHT11);
SoftwareSerial bt(2,3);
DS3231  rtc(SDA, SCL);
Time t;

unsigned long milis, milis2, milis3;
int pwm_lampu, pwm_fan, cont;
char x;
String i;
int POW, sam, eam, spm, epm, hpg, hsg, hml, hdf, TPG, TSG, TML, TDF;
float tpg, tsg, tml, tdf;
boolean kon_lampu_pagi, kon_lampu_sore;
boolean kon_heater, kon_humi;

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
  dht.begin();
  rtc.begin();
  lcd.begin();
  lcd.backlight();

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

  EEPROM.get(0, POW);
  EEPROM.get(5, sam);
  EEPROM.get(10, eam);
  EEPROM.get(15, spm);
  EEPROM.get(20, epm);
  EEPROM.get(25, TPG);
  EEPROM.get(30, TSG);
  EEPROM.get(35, TML);
  EEPROM.get(40, TDF);
  EEPROM.get(45, hpg);
  EEPROM.get(50, hsg);
  EEPROM.get(55, hml);
  EEPROM.get(60, hdf);

  tpg = TPG/10.00;
  tsg = TSG/10.00;
  tml = TML/10.00;
  tdf = TDF/10.00;

  pwm_fan = 255; fan; delay(500);
  pwm_fan = 100; fan;
}

void loop() {
  t = rtc.getTime();
  int jam = t.hour;

  if(!kon_heater and millis()-milis > 10000){
    kon_heater = true;
    milis = millis();
  }
  if(kon_heater and millis()-milis > 5000){
    kon_heater = false;
    milis = millis();
  }

  if(!kon_humi and millis()-milis2 > 5000){
    kon_humi = true;
    milis2 = millis();
  }
  if(kon_humi and millis()-milis2 > 1000){
    kon_humi = false;
    milis2 = millis();
  }
  
  if(sam != eam){kon_lampu_pagi = true;}
  else{kon_lampu_pagi = false;}

  if(spm != epm){kon_lampu_sore = true;}
  else{kon_lampu_sore = false;}

  if(kon_lampu_pagi){
    if(jam >= sam and jam < eam){
      pwm_lampu = map(POW, 0, 100, 0, 255);
      lamp;
    }
    else{pwm_lampu = 0; lamp;}
  }
  else if(kon_lampu_sore){
    if(jam >= spm and jam < epm){
      pwm_lampu = map(POW, 0, 100, 0, 255);
      lamp;
    }
    else{pwm_lampu = 0; lamp;}
  }
  else{pwm_lampu = 0; lamp;}

  if(jam >= 7 and jam < 10){atur_suhu(tpg);}
  else if(jam >= 12 and jam < 14){atur_suhu(tsg);}
  else if(jam >= 18 or jam < 6){atur_suhu(tml);}
  else{atur_suhu(tdf);}

  if(jam >= 7 and jam < 10){atur_humi(hpg);}
  else if(jam >= 12 and jam < 14){atur_humi(hsg);}
  else if(jam >= 18 or jam < 6){atur_humi(hml);}
  else{atur_humi(hdf);}

  fan;
  write_lcd();
  if(bt.available()){read_bluetooth();}
}

void atur_suhu(float dat_suhu){
  float t = dht.readTemperature();

  if(t < dat_suhu-2.00){
    pwm_fan = 255;
    if(kon_heater){heton;}
    else{hetof;}
  }
  else if(t < dat_suhu-1.00){
    pwm_fan = 150;
    hetof;
  }
  else{
    pwm_fan = 100;
    hetof;
  }

  if(t > dat_suhu+2.00){
    hetof; pwm_fan = 255;
  }
  else if(t > dat_suhu+1.00){
    hetof; pwm_fan = 180;
  }
}

void atur_humi(int dat_humi){
  float h = dht.readHumidity();
  
  if(h < dat_humi-2){
    if(kon_humi){humon; pwm_fan = 255;}
    else{humof; pwm_fan = 100;}
  }
  else if(h < dat_humi-1){
    pwm_fan = 150;
    humof;
  }
  else{
    pwm_fan = 100;
    humof;
  }

  if(h > dat_humi+2){
    humof; pwm_fan = 255;
  }
  else if(h > dat_humi+1){
    humof; pwm_fan = 180;
  }
}

void write_lcd(){
  if(millis()-milis3 > 5000){
    cont++;
    lcd.clear();
    if(cont > 8){cont = 0;}
    milis3 = millis();
  }
  
  if(cont == 0){
    lcd.setCursor(0,0);
    lcd.print("Suhu : ");
    lcd.print(temp);
    lcd.print((char)223);
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print("Kelembaban : ");
    lcd.print(humi);
    lcd.print("%");
  }
  else if(cont == 1){
    lcd.setCursor(0,0);
    lcd.print("Power : ");
    lcd.print(POW);
    lcd.print("%");
  }
  else if(cont == 2){
    lcd.setCursor(0,0);
    lcd.print("Start at ");
    lcd.print(sam);
    lcd.print(" AM");
    lcd.setCursor(0,1);
    lcd.print("  End at ");
    lcd.print(eam);
    lcd.print(" AM");
  }
  else if(cont == 3){
    lcd.setCursor(0,0);
    lcd.print("Start at ");
    lcd.print(spm);
    lcd.print(" PM");
    lcd.setCursor(0,1);
    lcd.print("  End at ");
    lcd.print(epm);
    lcd.print(" PM");
  }
  else if(cont == 4){
    lcd.setCursor(0,0);
    lcd.print("Suhu Pagi ");
    lcd.print(tpg,1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Suhu Siang ");
    lcd.print(tsg,1);
    lcd.print("C");
  }
  else if(cont == 5){
    lcd.setCursor(0,0);
    lcd.print("Suhu Mlm  ");
    lcd.print(tml,1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Suhu Dflt ");
    lcd.print(tdf,1);
    lcd.print((char)223);
    lcd.print("C");
  }
  else if(cont == 6){
    lcd.setCursor(0,0);
    lcd.print("Hum Pagi  ");
    lcd.print(hpg);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Hum Siang ");
    lcd.print(hsg);
    lcd.print("%");
  }
  else if(cont == 7){
    lcd.setCursor(0,0);
    lcd.print("Hum Malam   ");
    lcd.print(hml);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Hum Default ");
    lcd.print(hdf);
    lcd.print("%");
  }
  else if(cont == 8){
    String hari = rtc.getDOWStr();
    int i = (16 - hari.length()) / 2;
    lcd.setCursor(i,0);
    lcd.print(hari);
    lcd.setCursor(4,1);
    lcd.print(rtc.getTimeStr());
  }
}

void read_bluetooth(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Data Diterima");

  String kon = "";
  String dat = "";
  for(int i = 0; i <3; i++){
    char x = bt.read();
    kon += x;
  }
  while(bt.available()){
    char x = bt.read();
    dat += x;
  }

  if(kon == "POW"){
    EEPROM.put(0, dat.toInt());
    POW = dat.toInt();
  }
  else if(kon == "SAM"){
    EEPROM.put(5, dat.toInt());
    sam = dat.toInt();
  }
  else if(kon == "EAM"){
    EEPROM.put(10, dat.toInt());
    eam = dat.toInt();
  }
  else if(kon == "SPM"){
    EEPROM.put(15, dat.toInt());
    spm = dat.toInt();
  }
  else if(kon == "EPM"){
    EEPROM.put(20, dat.toInt());
    epm = dat.toInt();
  }
  else if(kon == "TPG"){
    EEPROM.put(25, dat.toInt());
    tpg = dat.toInt()/10.00;
  }
  else if(kon == "TSG"){
    EEPROM.put(30, dat.toInt());
    tsg = dat.toInt()/10.00;
  }
  else if(kon == "TML"){
    EEPROM.put(35, dat.toInt());
    tml = dat.toInt()/10.00;
  }
  else if(kon == "TDF"){
    EEPROM.put(40, dat.toInt());
    tdf = dat.toInt()/10.00;
  }
  else if(kon == "HPG"){
    EEPROM.put(45, dat.toInt());
    hpg = dat.toInt();
  }
  else if(kon == "HSG"){
    EEPROM.put(50, dat.toInt());
    hsg = dat.toInt();
  }
  else if(kon == "HML"){
    EEPROM.put(55, dat.toInt());
    hml = dat.toInt();
  }
  else if(kon == "HDF"){
    EEPROM.put(60, dat.toInt());
    hdf = dat.toInt();
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Data Disimpan");
  delay(1000);
  lcd.clear();
}
