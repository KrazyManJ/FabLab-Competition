#include <LiquidCrystal.h>

//
// DEFINING CONSTANTS
//

#define LCD_SIZE 16
#define SCROLL_TEXT_SPEED 50
#define TIME_DATE_SWITCH_DELAY 5000
//PINS
#define BUTTON_READ 2
#define BUTTON_OUTPUT 3
#define TEMP_INPUT_PIN A1
#define CONTRAST_OUTPUT A0

//
// LCD UTILS
//

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

byte degree[] = {
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

//
// SETUP
//

void setup(){
  pinMode(TEMP_INPUT_PIN,INPUT);
  pinMode(CONTRAST_OUTPUT,OUTPUT);
  pinMode(BUTTON_OUTPUT, OUTPUT);
  pinMode(BUTTON_READ, INPUT);
  
  analogWrite(CONTRAST_OUTPUT,0);
  lcd.createChar(0, degree);
  lcd.begin(16, 2);
//  Serial.begin(9600);
}

//
// LOOP
//

void loop(){
  bool pressed = false;
  bool switched = false;
  
  writeFadedText(0,"www.vassboskovice.cz");
  writeTimeDate(0,1);
  writeTemp(12,1);
  handleButton();
}

//
// BUTTON
//

bool pressed = false;
bool switched = false;

void handleButton(){
  int btn = digitalRead(BUTTON_READ);
  if (btn == 1 && !pressed){
    pressed = true;
    switched = !switched;
  	digitalWrite(BUTTON_OUTPUT,switched);
  }
  else if (btn == 0 && pressed) pressed = false;
}

//
// TEMP
//

void writeTemp(int x,int y){
  int temp = trackTemp();
  temp = temp < 100 ? temp : 99;
  temp = temp > -10 ? temp : -9;
  lcd.setCursor(x,y);
  lcd.print((String(temp).length() == 1 ? " " : "")+String(temp));
  lcd.write(byte(0));
  lcd.print("C");
}

int trackTemp(){
  return (int) map(analogRead(TEMP_INPUT_PIN),20,358,-40,125);
}

//
// DATE
//

long dt_switch = 0;
bool date_or_time = false;

void writeTimeDate(int x, int y){
  if (millis() > dt_switch+TIME_DATE_SWITCH_DELAY){
    dt_switch = millis();
    date_or_time = !date_or_time;
  }
  lcd.setCursor(x,y);
  lcd.print(date_or_time ? "30.03." : "19:00 ");
}

//
// FADED TEXT
//

long lastChanged = 0;
int pr = 0;

void writeFadedText(int y,String text){
  if (millis() >= lastChanged + SCROLL_TEXT_SPEED){
    lastChanged = millis();
    String out = repeat(" ",LCD_SIZE);
    if (pr <= LCD_SIZE)
      out = repeat(" ",LCD_SIZE-pr)+text.substring(0,pr);
    else if (pr <= text.length())
      out = text.substring(pr-LCD_SIZE,pr);
    else if (pr <= text.length()+LCD_SIZE) 
      out = text.substring(pr-LCD_SIZE)+repeat(" ",LCD_SIZE-text.substring(pr-LCD_SIZE).length());
    else pr = 0;
    lcd.setCursor(0,y);
    lcd.print(out);
    pr++;
  }
}

String repeat(String s, int times){
  int c = times;
  String r = "";
  while (c--) r+= s;
  return r;
}