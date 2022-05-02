#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <virtuabotixRTC.h>

//
// CONSTANTS
//

#define LCD_ADRESS 0x27
#define LCD_SIZE 16
#define LCD_DEFAULT_TEXT "www.vosassboskovice.cz"
#define SCROLL_TEXT_SPEED 800
#define TIME_DATE_SWITCH_DELAY 15000
#define COLON_BLINK_TIME 600

#define TEMP_INPUT_PIN 2

#define SD_CS_PIN 53
#define SPEAKER_PIN 9

#define RTC_CLK_PIN 5
#define RTC_DAT_PIN 6
#define RTC_RST_PIN 7

//
// MILLIS TIMER
//

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
		if (millis() > (starttime + delay))
		{
			starttime = millis();
			return true;
		}
		return false;
	}
};

//
// LCD UTILS
//

LiquidCrystal_I2C lcd(LCD_ADRESS, LCD_SIZE, 2);

byte warning[8] = {
	B00100,
	B00100,
	B01110,
	B01010,
	B01010,
	B11111,
	B11011,
	B11111};
byte temp[8] = {
	B00100,
	B01010,
	B01010,
	B01010,
	B01010,
	B10001,
	B10001,
	B01110,
};

void initializeLCD()
{
	lcd.init();
	lcd.backlight();
	lcd.createChar(0, warning);
	lcd.createChar(1, temp);
}

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

DHT dht(TEMP_INPUT_PIN, DHT11);

void tempInitialize()
{
	pinMode(TEMP_INPUT_PIN, INPUT);
	dht.begin();
}

MillisTimer warningBlink(800);
bool warnB;

int gatherTempWithBound()
{
	int temp = dht.readTemperature();
	temp = temp < 100 ? temp : 99;
	return temp > -10 ? temp : -9;
}

void writeTemp(int x, int y)
{
	if (isnan(dht.readTemperature()))
	{
		lcd.setCursor(x, y);
		lcd.print(" ");
		if (warningBlink.isReady())
			warnB = !warnB;
		lcd.write(warnB ? 0 : ' ');
		lcd.write(1);
		lcd.print(" ");
		return;
	}
	int temp = gatherTempWithBound();
	lcd.setCursor(x, y);
	lcd.print((String(temp).length() == 1 ? " " : "") + String(temp));
	lcd.print((char)223);
	lcd.print("C");
}

//
// TIME AND DATE
//

virtuabotixRTC myRTC(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);

void updateTime(int year, int month, int day, int hour, int minutes)
{
	//seconds, minutes, hours, day of the week, day of the month, month, year
	myRTC.setDS1302Time(0, minutes, hour, 0, day, month, year);
}

bool date_or_time = false;
bool colon = false;

MillisTimer colon_timer(COLON_BLINK_TIME);
MillisTimer date_time(TIME_DATE_SWITCH_DELAY);

String formatDate()
{
	return numberStr(myRTC.dayofmonth, 2) + "." + numberStr(myRTC.month, 2) + "." + String(myRTC.year);
}
String formatTime(bool colon)
{
	return numberStr(myRTC.hours, 2) + (colon ? ":" : " ") + numberStr(myRTC.minutes, 2);
}

void writeTimeDate(int x, int y)
{
	myRTC.updateTime();
	if (date_time.isReady())
		date_or_time = !date_or_time;
	if (!date_or_time && colon_timer.isReady())
		colon = !colon;
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

void initializeFadedText(bool hasSD)
{
	if (hasSD)
	{
		File lcdTextFile = SD.open("lcd.txt");
		if (lcdTextFile)
		{
			while (lcdTextFile.available())
				text = lcdTextFile.readString();
			lcdTextFile.close();
			return;
		}
	}
	text = LCD_DEFAULT_TEXT;
}

void writeFadedText()
{
	if (text == "")
	{
		lcd.setCursor(0, 0);
		lcd.print(" ");
		lcd.write(0);
		lcd.print(" Null title ");
		lcd.write(0);
		lcd.print(" ");
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
		lcd.setCursor(0, 0);
		lcd.print(out);
		pr++;
	}
}

//
// SPEAKER
//

TMRpcm tmrpcm;

void initializeSpeaker()
{
	tmrpcm.speakerPin = 46;
	tmrpcm.volume(1);
	tmrpcm.quality(1);
	// tmrpcm.play("test");
}

//
// SETUP
//

void setup()
{
	Serial.begin(9600);
	tempInitialize();
	initializeLCD();
	bool hasSD = SD.begin(SD_CS_PIN);
	initializeFadedText(hasSD);
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
