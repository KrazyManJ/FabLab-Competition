#include <LiquidCrystal.h>

// C++ code
//

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

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

void setup()
{
  analogWrite(6,0);
  lcd.createChar(0, degree);
  lcd.begin(16, 2);
  Serial.begin(9600);
}

int pr = 0;

void loop()
{
  lcd.clear();
  writeFadedText(0,"www.vassboskovice.cz");
  writeTemp(12,1);
  delay(500);
}

int trackTemp(){
  return (int) map(analogRead(A0),20,358,-40,125);
}

void writeTemp(int x,int y){
  lcd.setCursor(x, y);
  lcd.print(""+String(trackTemp()));
  lcd.write(byte(0));
  lcd.print("C");
}

void writeFadedText(int y,String text){
  if (pr <= text.length()){
  	lcd.setCursor(0,y);
    lcd.print(text.substring(text.length()-pr));
  }
  else if (pr < text.length()+16){
  	lcd.setCursor(floor(pr-text.length()),y);
    lcd.print(text);
  }
  else pr = 0;
  pr++;
}