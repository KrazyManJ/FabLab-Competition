#include <LiquidCrystal.h>

//
// DEFINING
//

#define LCD_SIZE 16
#define SCROLL_TEXT_SPEED 100

//
// LCD UTILS
//

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

byte degree[] = {
  B11100,
  B10100,
  B11100,
  B00000,
  B00011,
  B00100,
  B00100,
  B00011
};

//
// SETUP
//

void setup(){
  pinMode(7,OUTPUT);
  pinMode(8,INPUT);
  analogWrite(6,0);
  analogWrite(A1,255);
  lcd.createChar(0, degree);
  lcd.begin(16, 2);
  Serial.begin(9600);
}

//
// LOOP
//

void loop(){
  writeFadedText(0,"www.vassboskovice.cz");
  writeTimeDate(0,1);
  writeTemp(13,1);
  handleButton();
}

//
// BUTTON
//

bool pressed = false;
bool switched = false;

void handleButton(){
  int btn = digitalRead(8);
  Serial.println(btn);
  if (btn == 1 && !pressed){
    pressed = true;
    switchLED();
  }
  else if (btn == 0 && pressed) pressed = false;
}

void switchLED(){
  switched = !switched;
  digitalWrite(7,switched);
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
}

int trackTemp(){
  return (int) map(analogRead(A0),20,358,-40,125);
}

//
// DATE
//

void writeTimeDate(int x, int y){
  lcd.setCursor(x,y);
  lcd.print("30.03.12:05");
}

//
// FADED TEXT
//

float lastChanged = 0;
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