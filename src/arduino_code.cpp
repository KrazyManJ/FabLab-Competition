#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <virtuabotixRTC.h>

#define LCD_ADRESS 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define LCD_DEFAULT_TEXT "www.vosassboskovice.cz"
#define SCROLL_TEXT_SPEED 800
#define TIME_DATE_SWITCH_DELAY 15000
#define COLON_BLINK_TIME 600

#define TEMP_INPUT_PIN 2
#define TEMP_MODULE_TYPE DHT11

#define SD_CS_PIN 53
#define SPEAKER_PIN 9

#define RTC_CLK_PIN 5
#define RTC_DAT_PIN 6
#define RTC_RST_PIN 7

//=====================================================================================
//											MILLIS TIMER
//=====================================================================================

/**
 * Class for handling timer using millis() function
 **/
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

//=====================================================================================
//											LCD
//=====================================================================================

LiquidCrystal_I2C lcd(LCD_ADRESS, LCD_WIDTH, LCD_HEIGHT);

byte warningChar[8] = {0x04, 0x04, 0x0E, 0x0A, 0x0A, 0x1F, 0x1B, 0x1F};
byte tempChar[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x11, 0x11, 0x0E};


/**
 * Initialize LCD display and register custom characters
 **/
void initializeLCD()
{
	lcd.init();
	lcd.backlight();
	lcd.createChar(0, warningChar);
	lcd.createChar(1, tempChar);
}

//=====================================================================================
//										UTILS METHODS
//=====================================================================================

/**
 * Repeat sequence of string 
 **/
String repeat(String s, int times)
{
	int c = times;
	String r = "";
	while (c--)
		r += s;
	return r;
}

/**
 * Format non-decimal as string with specific length
 * 
 * Example:
 * - numberStr(18,4) => 0018
 * - numberStr(3,2) => 03
 **/
String numberStr(int i, int length)
{
	return repeat("0", length - String(i).length()) + String(i);
}

//=====================================================================================
//										TEMPERATURE
//=====================================================================================

DHT dht(TEMP_INPUT_PIN, TEMP_MODULE_TYPE);

/**
 * Initialize temperature sensor for future usage
 **/
void tempInitialize()
{
	pinMode(TEMP_INPUT_PIN, INPUT);
	dht.begin();
}

MillisTimer warningBlink(800);
bool warnB;

/**
 * Gets temperature with minimum value of -9°C and maximum of 99°C
 * 
 * This is due to size of LCD.
 **/
int gatherBoundedTemp()
{
	int temp = dht.readTemperature();
	temp = temp < 100 ? temp : 99;
	return temp > -10 ? temp : -9;
}

/**
 * Write temp at specific location.
 * 
 * If sensor is disconnected, then it will write temp warning text!
 **/
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
	int temp = gatherBoundedTemp();
	lcd.setCursor(x, y);
	lcd.print((String(temp).length() == 1 ? " " : "") + String(temp));
	lcd.print((char)223);
	lcd.print("C");
}

//=====================================================================================
//										TIME AND DATE
//=====================================================================================

virtuabotixRTC myRTC(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);

/**
 * Sets time to specific inputted data.
 **/
void updateTime(int year, int month, int day, int hour, int minutes)
{
	//seconds, minutes, hours, day of the week, day of the month, month, year
	myRTC.setDS1302Time(0, minutes, hour, 0, day, month, year);
}

bool date_or_time = false;
bool colon = false;

MillisTimer colon_timer(COLON_BLINK_TIME);
MillisTimer date_time(TIME_DATE_SWITCH_DELAY);

/**
 * Formats date with data from RTC
 **/
String formatDate()
{
	return numberStr(myRTC.dayofmonth, 2) + "." + numberStr(myRTC.month, 2) + "." + String(myRTC.year);
}
/**
 * Formats time with data from RTC
 **/
String formatTime(bool colon)
{
	return numberStr(myRTC.hours, 2) + (colon ? ":" : " ") + numberStr(myRTC.minutes, 2);
}

/**
 * Writes time or date specified by MillisTimer switch
 **/
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

//=====================================================================================
//										SCROLLING TEXT
//=====================================================================================

String text = "";
MillisTimer fadeTimer(SCROLL_TEXT_SPEED);
unsigned int pr = 0;

/**
 * Reads data from SD from file "lcd.txt" and initialize text, which will scroll over.
 * If there is no file "lcd.txt" or there is no SD card, then it will be used LCD_DEFAULT_TEXT
 * @param hasSD if sd is loaded or not
 **/
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

/**
 * Write faded text to first line of LCD
 **/
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
		String out = repeat(" ", LCD_WIDTH);
		if (pr <= LCD_WIDTH)
			out = repeat(" ", LCD_WIDTH - pr) + text.substring(0, pr) + repeat(" ", text.length() - pr + LCD_WIDTH);
		else if (pr <= text.length())
			out = text.substring(pr - LCD_WIDTH, pr);
		else if (pr <= text.length() + LCD_WIDTH)
			out = text.substring(pr - LCD_WIDTH) + repeat(" ", LCD_WIDTH - text.substring(pr - LCD_WIDTH).length());
		else
			pr = 0;
		lcd.setCursor(0, 0);
		lcd.print(out);
		pr++;
	}
}

//=====================================================================================
//											SPEAKER
//=====================================================================================

TMRpcm tmrpcm;

void initializeSpeaker()
{
	tmrpcm.speakerPin = 46;
	pinMode(46, OUTPUT);
	tmrpcm.setVolume(50);
	tmrpcm.play((char *)"test");
	Serial.println(tmrpcm.isPlaying());
}

//=====================================================================================
//										SETUP & LOOP
//=====================================================================================

void setup()
{
	Serial.begin(9600);
	tempInitialize();
	initializeLCD();
	bool hasSD = SD.begin(SD_CS_PIN);
	initializeFadedText(hasSD);
	initializeSpeaker();
}

void loop()
{
	writeFadedText();
	writeTimeDate(0, 1);
	writeTemp(12, 1);
	delay(1);
}
