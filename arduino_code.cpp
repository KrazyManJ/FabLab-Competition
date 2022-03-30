#include <LiquidCrystal.h>

//
// CONSTANTS
//

#define LCD_SIZE 16
#define SCROLL_TEXT_SPEED 50
#define TIME_DATE_SWITCH_DELAY 5000

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
long buttonTask = 0;
byte pinOn = 0;

void handleButton(){
  if (!pressed){
  	for (int i = 0; i < sizeof(buttons)/sizeof(buttonLedBind); i++){
      if (digitalRead(buttons[i].input) == HIGH){
        pressed = true;
        pinOn = buttons[i].output;
      	analogDigitalWrite(buttons[i].output,true);
        buttonTask = millis()+buttons[i].milliTime;
      }
    }
  }
  else{
    if (millis() > buttonTask){
      pressed = false;
      analogDigitalWrite(pinOn,false);
    }
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
  lcd.print(date_or_time ? "30.03.2022" : "19:00     ");
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