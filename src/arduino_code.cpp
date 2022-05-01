#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

//
// CONSTANTS
//

#define LCD_SIZE 16
#define SCROLL_TEXT_SPEED 800
#define TIME_DATE_SWITCH_DELAY 10000
#define DOT_BLINK_TIME 600
#define CONTRAST_OUTPUT_PIN A0

#define TEMP_INPUT_PIN A1

#define SD_CS_PIN 10
#define SPEAKER_PIN 11

#define BUILDING_A_BUTTON_PIN
#define BUILDING_A_LED_PIN
#define BUILDING_B_BUTTON_PIN
#define BUILDING_B_LED_PIN
#define BUILDING_C_BUTTON_PIN
#define BUILDING_C_LED_PIN
#define BUILDING_D_BUTTON_PIN
#define BUILDING_D_LED_PIN

//
// LCD UTILS
//

LiquidCrystal_I2C lcd(0x27, LCD_SIZE, 2);

class MillisTimer
{
	long starttime;
	int delay;

public:
	MillisTimer(int d)
	{
		delay = d;
		starttime = millis();
	}
	bool isReady()
	{
		return millis() > (starttime + delay);
	}
	void reset()
	{
		starttime = millis();
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

int trackTemp()
{
	return (int)map(analogRead(TEMP_INPUT_PIN), 20, 358, -40, 125);
}

void writeTemp(int x, int y)
{
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
int month = 4;
int year = 2022;
int hour = 12;
int minute = 0;

bool date_or_time = false;
bool double_dot = false;

MillisTimer dot(DOT_BLINK_TIME);
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
	if (date_time.isReady())
	{
		date_time.reset();
		date_or_time = !date_or_time;
	}
	if (!date_or_time && dot.isReady())
	{
		dot.reset();
		double_dot = !double_dot;
	}
	lcd.setCursor(x, y);
	lcd.print(date_or_time
				  ? formatDate()
				  : formatTime(double_dot) + repeat(" ", formatDate().length() - formatTime(double_dot).length()));
}

//
// FADED TEXT
//

String text = "";
MillisTimer fadeTimer(SCROLL_TEXT_SPEED);
int pr = 0;

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

void writeFadedText(int y)
{
	if (text == "")
	{
		lcd.setCursor(0, y);
		lcd.print(" Err: Null text");
		return;
	}
	if (fadeTimer.isReady())
	{
		fadeTimer.reset();
		String out = repeat(" ", LCD_SIZE);
		if (pr <= LCD_SIZE)
			out = repeat(" ", LCD_SIZE - pr) + text.substring(0, pr) + repeat(" ", text.length() - pr + LCD_SIZE);
		else if (pr <= text.length())
			out = text.substring(pr - LCD_SIZE, pr);
		else if (pr <= text.length() + LCD_SIZE)
			out = text.substring(pr - LCD_SIZE) + repeat(" ", LCD_SIZE - text.substring(pr - LCD_SIZE).length());
		else
			pr = 0;
		lcd.setCursor(0, y);
		lcd.print(out);
		pr++;
	}
}

//
// SETUP
//

void setup()
{
	pinMode(TEMP_INPUT_PIN, INPUT);
	pinMode(CONTRAST_OUTPUT_PIN, OUTPUT);

	analogWrite(CONTRAST_OUTPUT_PIN, 0);
	lcd.begin(LCD_SIZE, 2);
	lcd.backlight();

	Serial.begin(9600);
	
	if (SD.begin(SD_CS_PIN)) initializeFadedText();
	else
		text = "www.vassboskovice.cz";
}

//
// LOOP
//

void loop()
{
	writeFadedText(0);
	writeTimeDate(0, 1);
	writeTemp(12, 1);
	delay(1);
}
