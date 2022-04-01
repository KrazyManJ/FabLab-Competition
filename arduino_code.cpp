#include <LiquidCrystal.h>

//
// CONSTANTS
//

#define LCD_SIZE 16
#define SCROLL_TEXT_SPEED 50
#define TIME_DATE_SWITCH_DELAY 5000
#define DOT_BLINK_TIME 80

#define TEMP_INPUT_PIN A1
#define CONTRAST_OUTPUT_PIN A0

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
  pinMode(CONTRAST_OUTPUT_PIN,OUTPUT);
  
  analogWrite(CONTRAST_OUTPUT_PIN,0);
  lcd.createChar(0, degree);
  lcd.begin(16, 2);
  Serial.begin(9600);
  registerButtons();
}

//
// LOOP
//

void loop(){
  writeFadedText(0,"www.vosassboskovice.cz");
  writeTimeDate(0,1);
  writeTemp(12,1);
  handleButton();
  delay(1);
}

class MillisTimer {
  long starttime;
  int delay;
  public:
    MillisTimer(int d){
      delay = d;
      starttime = millis();
    }
    bool isReady(){
      return millis() > (starttime + delay);
    }
    void reset(){
      starttime = millis();
    }
};

//
// BUTTONS
//

typedef struct {
  int input;
  byte output;
  long milliTime;
} buttonLedBind;

const buttonLedBind buttons[] {
  {2,A5,5000},
  {3,A4,4000},
  {4,A3,1800},
  {5,A2,2600},
};

void registerButtons(){
  for (int i = 0; i < sizeof(buttons)/sizeof(buttonLedBind); i++){
  	pinMode(buttons[i].input, INPUT);
    pinMode(buttons[i].output, OUTPUT);
  }
}

bool pressed = false;
MillisTimer buttonTimer(0);
byte pinOn = NULL;

void handleButton(){
  if (!pressed){
  	for (int i = 0; i < sizeof(buttons)/sizeof(buttonLedBind); i++){
      if (digitalRead(buttons[i].input) == HIGH){
        pressed = true;
        pinOn = buttons[i].output;
        buttonTimer = MillisTimer(buttons[i].milliTime);
      	analogDigitalWrite(buttons[i].output,true);
      }
    }
  }
  else if (buttonTimer.isReady()){
    pressed = false;
    analogDigitalWrite(pinOn,false);
  }
}

void analogDigitalWrite(byte pin, bool v){
  analogWrite(pin,v ? 255 : 0);
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
// TIME AND DATE
//

int day = 1;
int month = 4;
int year = 2022;
int hour = 12;
int minute = 0;


bool date_or_time = false;
bool double_dot = false;

MillisTimer dot(DOT_BLINK_TIME);
MillisTimer date_time(TIME_DATE_SWITCH_DELAY);

void writeTimeDate(int x, int y){
  if (date_time.isReady()){
    date_time.reset();
    date_or_time = !date_or_time;
  }
  if (!date_or_time && dot.isReady()){
    dot.reset();
    double_dot = !double_dot;
  }
  lcd.setCursor(x,y);
  lcd.print(date_or_time 
            ? formatDate() 
            : formatTime(double_dot)+repeat(" ",formatDate().length()-formatTime(double_dot).length()));
}

String formatDate(){
  return numberStr(day,2)+"."+numberStr(month,2)+"."+String(year);
}
String formatTime(bool colon){
  return numberStr(hour,2)+(colon ? ":" : " ")+numberStr(minute,2);
}

//
// FADED TEXT
//

MillisTimer fadeTimer(SCROLL_TEXT_SPEED);
int pr = 0;

void writeFadedText(int y,String text){
  if (fadeTimer.isReady()){
    fadeTimer.reset();
    String out = repeat(" ",LCD_SIZE);
    if (pr <= LCD_SIZE) out = repeat(" ",LCD_SIZE-pr)+text.substring(0,pr);
    else if (pr <= text.length()) out = text.substring(pr-LCD_SIZE,pr);
    else if (pr <= text.length()+LCD_SIZE) 
      out = text.substring(pr-LCD_SIZE)+repeat(" ",LCD_SIZE-text.substring(pr-LCD_SIZE).length());
    else pr = 0;
    lcd.setCursor(0,y);
    lcd.print(out);
    pr++;
  }
}

//
// UTILS METHODS
//

String repeat(String s, int times){
  int c = times;
  String r = "";
  while (c--) r+= s;
  return r;
}

String numberStr(int i, int length){
  return repeat("0",length - String(i).length())+String(i);
}