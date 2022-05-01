#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

//
// CONSTANTS
//

#define LCD_SIZE 16
#define LCD_DEFAULT_TEXT "www.vosassboskovice.cz"
#define SCROLL_TEXT_SPEED 800
#define TIME_DATE_SWITCH_DELAY 10000
#define COLON_BLINK_TIME 600

#define TEMP_INPUT_PIN A1

#define SD_CS_PIN 10
#define SPEAKER_PIN 9

//
// LCD UTILS
//

LiquidCrystal_I2C lcd(0x27, LCD_SIZE, 2);

class MillisTimer
{
	unsigned long starttime;
	unsigned int delay;

public:
	MillisTimer(int d)
	{
		delay = d;
		starttime = millis();
	}
	bool isReady()
	{
		if (millis() > (starttime + delay)) {
			starttime = millis();
			return true;
		}
		return false;
	}
};

//
// UTILS METHODS
//

String repeat(String s, int times)
{
	int c = times;
	String r = "";
	while (c--)
		r += s;
	return r;
}

String numberStr(int i, int length)
{
	return repeat("0", length - String(i).length()) + String(i);
}

//
// TEMP
//


byte warning[8] = {
	B00100,
	B00100,
	B01110,
	B01010,
	B01010,
	B11111,
	B11011,
	B11111
};
byte temp[8] =                    
{            
	B00100,           
	B01010,           
	B01010,           
	B01010,           
	B01010,           
	B10001,           
	B10001,           
	B01110,            
};
MillisTimer warningBlink(800);
bool warnB;

int trackTemp()
{
	return (int)map(analogRead(TEMP_INPUT_PIN), 20, 358, -40, 125);
}

void writeTemp(int x, int y)
{
	if (analogRead(TEMP_INPUT_PIN) == 0){
		lcd.setCursor(x, y);
		lcd.print(" ");
		if (warningBlink.isReady()) warnB = !warnB;
		lcd.write(warnB ? 0 : ' ');
		lcd.write(1);
		lcd.print(" ");
		return;
	}
	int temp = trackTemp();
	temp = temp < 100 ? temp : 99;
	temp = temp > -10 ? temp : -9;
	lcd.setCursor(x, y);
	lcd.print((String(temp).length() == 1 ? " " : "") + String(temp));
	lcd.print((char)223);
	lcd.print("C");
}

//
// TIME AND DATE
//

int day = 1;
int month = 5;
int year = 2022;
int hour = 17;
int minute = 40;

bool date_or_time = false;
bool colon = false;

MillisTimer colon_timer(COLON_BLINK_TIME);
MillisTimer date_time(TIME_DATE_SWITCH_DELAY);

String formatDate()
{
	return numberStr(day, 2) + "." + numberStr(month, 2) + "." + String(year);
}
String formatTime(bool colon)
{
	return numberStr(hour, 2) + (colon ? ":" : " ") + numberStr(minute, 2);
}

void writeTimeDate(int x, int y)
{
	if (date_time.isReady()) date_or_time = !date_or_time;
	if (!date_or_time && colon_timer.isReady()) colon = !colon;
	lcd.setCursor(x, y);
	lcd.print(date_or_time
				  ? formatDate()
				  : formatTime(colon) + repeat(" ", formatDate().length() - formatTime(colon).length()));
}

//
// FADED TEXT
//

String text = "";
MillisTimer fadeTimer(SCROLL_TEXT_SPEED);
unsigned int pr = 0;

void initializeFadedText()
{
	File myFile = SD.open("lcd.txt");
	if (myFile)
	{
		while (myFile.available()){
			text = myFile.readString();
		}
		myFile.close();
	}
}

void writeFadedText()
{
	if (text == "")
	{
		lcd.setCursor(0, 1);
		lcd.print(" Err: Null text");
		return;
	}
	if (fadeTimer.isReady())
	{
		String out = repeat(" ", LCD_SIZE);
		if (pr <= LCD_SIZE)
			out = repeat(" ", LCD_SIZE - pr) + text.substring(0, pr) + repeat(" ", text.length() - pr + LCD_SIZE);
		else if (pr <= text.length())
			out = text.substring(pr - LCD_SIZE, pr);
		else if (pr <= text.length() + LCD_SIZE)
			out = text.substring(pr - LCD_SIZE) + repeat(" ", LCD_SIZE - text.substring(pr - LCD_SIZE).length());
		else
			pr = 0;
		lcd.setCursor(0, 1);
		lcd.print(out);
		pr++;
	}
}

//
// SETUP
//

void setup()
{
	Serial.begin(9600);

	pinMode(TEMP_INPUT_PIN, INPUT);

	lcd.begin(LCD_SIZE, 2);
	lcd.backlight();
	lcd.createChar(0, warning);
	lcd.createChar(1, temp);

	if (SD.begin(SD_CS_PIN)) initializeFadedText();
	else text = LCD_DEFAULT_TEXT;
}

//
// LOOP
//

void loop()
{
	writeFadedText();
	writeTimeDate(0, 1);
	writeTemp(12, 1);
	delay(1);
}
