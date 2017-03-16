/***************************************************

KegBot v.Alpha
Aaron Garcia
Head Brewer
Small Brewpub
Dallas, TX
aaron@smallbrewpub.com

Additional Help by Bo Bartlett

 ****************************************************/

// INCLUDES

// #include <EEPROM.h>
// #include "writeAnything.h"
// #include <Wire.h>
// #include <avr/wdt.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "RTClib.h"
#include "Adafruit_LEDBackpack.h"
#include "Small_Cycle.h"

// Put SCK, MOSI, SS pins into output mode
// also put SCK, MOSI into LOW state, and SS into HIGH state.

// Pins For the TFT SCREEN

#define TFT_SCLK 13  // SainSmart: SCL or CLK (Clock) is SCK (Serial Clock)
#define TFT_MOSI 11  // SainSmart: SDA or MOSI or DIN (Data In) is MOSI (Master Out, Slave In)
#define TFT_CS   7  // SainSmart: CS (Chip Select) is SS (Slave Select)
#define TFT_DC   8  // SainSmart: RS/DC (MISO?)
#define TFT_RST  9  // SainSmart: RES --- maybe save this for another output

// Setup

// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR false

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS_time   0x70
#define DISPLAY_ADDRESS_status   0x71

Adafruit_7segment status_display = Adafruit_7segment();
Adafruit_7segment time_display = Adafruit_7segment();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC);
Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

RTC_DS1307 RTC;

#ifndef _BV
	#define _BV(bit) (1<<(bit))
#endif

// uint8_t counter = 0;

DateTime startTime;

// Pin Mapping

const int startBtn = A0;            // Start Cycle Pushbutton

const byte pump = A1;                // Relay 1
const byte valve1 = A2;              // Relay 2
const byte valve2 = A3;              // Relay 3
const byte valve3 = 2;               // Relay 4
const byte valve4 = 5;               // Relay 5
const byte co2 = 3;                	 // Relay 6

// const int resetBtn = 4;           // ...does 4 even work?
const byte buzzer = 6;               // buzzer connected to D6

// int kegsWashedSession;


/*
Small_Cycle Cycle01 = Small_Cycle(18); // Purge to drain
//Cycle01.display = "Stage 1";
Small_Cycle Cycle02 = Small_Cycle(10); // Purge with co2 1
Small_Cycle Cycle03 = Small_Cycle(20); // Rinse 1
Small_Cycle Cycle04 = Small_Cycle(25); // Purge with co2 2
Small_Cycle Cycle05 = Small_Cycle(20); // Rinse 2
Small_Cycle Cycle06 = Small_Cycle(30); // Purge with co2 3
Small_Cycle Cycle07 = Small_Cycle(45); // Recir with Acid
Small_Cycle Cycle08 = Small_Cycle(35); // Purge acid back to tank with co2
Small_Cycle Cycle09 = Small_Cycle(10); // Rinse 3
Small_Cycle Cycle10 = Small_Cycle(35); // Purge with co2 4
Small_Cycle Cycle11 = Small_Cycle(10); // Pressurize keg
Small_Cycle Cycle12 = Small_Cycle(1);  // Stop/Reset
*/
const int cyclesCount = 12;
Small_Cycle cycles[cyclesCount];
/*Small_Cycle cycles[] = {
	Cycle01,
	Cycle02,
	Cycle03,
	Cycle04,
	Cycle05,
	Cycle06,
	Cycle07,
	Cycle08,
	Cycle09,
	Cycle10,
	Cycle11,
	Cycle12
};
*/


// in seconds

const byte Stage1_duration = 18;	// Purge to drain
const byte Stage2_duration = 10;	// Purge with co2 1
const byte Stage3_duration = 20;	// Rinse 1
const byte Stage4_duration = 25;	// Purge with co2 2
const byte Stage5_duration = 20;	// Rinse 2
const byte Stage6_duration = 30;	// Purge with co2 3
const byte Stage7_duration = 45;	// Recir with Acid
const byte Stage8_duration = 35;	// Purge acid back to tank with co2
const byte Stage9_duration = 10;	// Rinse 3
const byte Stage10_duration = 35;	// Purge with co2 4
const byte Stage11_duration = 10;	// Pressurize keg
const byte Stage12_duration = 1;	// Stop/Reset

const int Stage1 = Stage1_duration;
const int Stage2 = Stage1 + Stage2_duration;
const int Stage3 = Stage2 + Stage3_duration;
const int Stage4 = Stage3 + Stage4_duration;
const int Stage5 = Stage4 + Stage5_duration;
const int Stage6 = Stage5 + Stage6_duration;
const int Stage7 = Stage6 + Stage7_duration;
const int Stage8 = Stage7 + Stage8_duration;
const int Stage9 = Stage8 + Stage9_duration;
const int Stage10 = Stage9 + Stage10_duration;
const int Stage11 = Stage10 + Stage11_duration;
const int Stage12 = Stage11 + Stage12_duration;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long nowSeconds;
unsigned long startSeconds;

String comdata = "";
int mark=0;
//store the current time data
int rtc[7];
//store the set time data
byte rr[7];

void setup(void) {
	Small_Cycle Cycle01 = Small_Cycle(); // Purge to drain
		Cycle01.duration(18);
		Cycle01.display("Stage 1");
	Small_Cycle Cycle02 = Small_Cycle(); // Purge with co2 1
		Cycle02.duration(10);
		Cycle02.display("Stage 1");
	Small_Cycle Cycle03 = Small_Cycle(); // Rinse 1
		Cycle03.duration(20);
		Cycle03.display("Stage 1");
	Small_Cycle Cycle04 = Small_Cycle(); // Purge with co2 2
		Cycle04.duration(25);
		Cycle04.display("Stage 1");
	Small_Cycle Cycle05 = Small_Cycle(); // Rinse 2
		Cycle05.duration(20);
		Cycle05.display("Stage 1");
	Small_Cycle Cycle06 = Small_Cycle(); // Purge with co2 3
		Cycle06.duration(30);
		Cycle06.display("Stage 1");
	Small_Cycle Cycle07 = Small_Cycle(); // Recir with Acid
		Cycle07.duration(45);
		Cycle07.display("Stage 1");
	Small_Cycle Cycle08 = Small_Cycle(); // Purge acid back to tank with co2
		Cycle08.duration(35);
		Cycle08.display("Stage 1");
	Small_Cycle Cycle09 = Small_Cycle(); // Rinse 3
		Cycle09.duration(10);
		Cycle09.display("Stage 1");
	Small_Cycle Cycle10 = Small_Cycle(); // Purge with co2 4
		Cycle10.duration(35);
		Cycle10.display("Stage 1");
	Small_Cycle Cycle11 = Small_Cycle(); // Pressurize keg
		Cycle11.duration(10);
		Cycle11.display("Stage 1");
	Small_Cycle Cycle12 = Small_Cycle();  // Stop/Reset
		Cycle12.duration(1);
		Cycle12.display("Stage 1");

	cycles[0]  = Cycle01;
	cycles[1]  = Cycle02;
	cycles[2]  = Cycle03;
	cycles[3]  = Cycle04;
	cycles[4]  = Cycle05;
	cycles[5]  = Cycle06;
	cycles[6]  = Cycle07;
	cycles[7]  = Cycle08;
	cycles[8]  = Cycle09;
	cycles[9]  = Cycle10;
	cycles[10] = Cycle11;
	cycles[11] = Cycle12;


// addresses for 4 digit readouts

	matrix.begin(0x70);  // pass in the address
	matrix.begin(0x71);  // pass in the address

// DEFINING PINS FOR INPUT/OUTPUT

	pinMode(pump, OUTPUT);
	pinMode(valve1, OUTPUT);
	pinMode(valve2, OUTPUT);
	pinMode(valve3, OUTPUT);
	pinMode(valve4, OUTPUT);
	pinMode(co2, OUTPUT);

	pinMode(buzzer, OUTPUT);

	// pinMode(resetBtn, INPUT);
	pinMode(startBtn, INPUT);

	pinMode(pump, HIGH );
	pinMode(valve1, HIGH );
	pinMode(valve2, HIGH );
	pinMode(valve3, HIGH );
	pinMode(valve4, HIGH );
	pinMode(co2, HIGH );

	// pinMode(resetBtn, LOW );
	pinMode(startBtn, LOW );

	pinMode(buzzer, HIGH );

	doCo2Pressure();
	digitalWrite(buzzer, 1 );

// clock bizness

	Wire.begin();

	status_display.begin(DISPLAY_ADDRESS_status);
	time_display.begin(DISPLAY_ADDRESS_time);

	time_display.setBrightness(2);
	time_display.writeDisplay();

	status_display.setBrightness(2);
	status_display.drawColon(false);
	status_display.writeDigitRaw(0,109);
	status_display.writeDigitRaw(1,64);
	status_display.writeDigitRaw(3,64);
	status_display.writeDisplay();

		// Manual Clock Set, it will set to this on upload, should only have to do this once.
		// Wire.beginTransmission(0x68); // address DS3231
		// Wire.write(0x00); // select register

		// Wire.write(0x15); // seconds (BCD)
		// Wire.write(0x54); // minutes (BCD)
		// Wire.write(0x11); // hours (BCD)
		// Wire.write(0x04); // day of week (I use Mon=1 .. Sun=7)
		// Wire.write(0x12); // day of month (BCD)
		// Wire.write(0x01); // month (BCD)
		// Wire.write(0x17); // year (BCD, two digits)

		// Wire.endTransmission();

	RTC.begin(); // initialize a tinyRTC chip

	tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

	tft.fillScreen(ST7735_BLACK); // Clear screen
	tft.setTextSize(1);

	tft.setTextColor(ST7735_RED);
	tft.setCursor(5,10);
	tft.println("Small Brewpub");
	tft.setCursor(5,20);
	tft.println("Keg Washer");
	tft.setCursor(5,30);
	tft.setTextColor(ST7735_WHITE);
	tft.println("v.Alpha");

}

void loop() {

	DateTime now = RTC.now(); //GRAB DATE AND TIME FROM RTC
	nowSeconds = (now.hour() * 60 * 60) + (now.minute() * 60) + now.second();

	//for (int thisCycle = cyclesCount - 1; thisCycle >= 0; thisCycle--) {
	for (int thisCycle = 0; thisCycle <= cyclesCount-1; thisCycle++) {
		if( !cycles[thisCycle].started() ) {
			cycles[thisCycle].started(true);
			// doTheCycleThing();
			// Start the cycle
			break;
		}
		if( cycles[thisCycle].started() && !cycles[thisCycle].completed() ) {
			// Check to see if the cycle is still running
			Serial.println('yep ' + thisCycle);
			// if it is complete and is the last, then do all of the cleanup
		}
	}

	displayTime(now);

// tft.setCursor(7,148);
// tft.setTextColor(ST7735_RED,ST7735_BLACK);
// tft.print("Kegs Washed: ");
// tft.print(kegsWashedSession);

// button stuff

// read the state of the switch into a local variable:
int reading_start = digitalRead(startBtn);

// int reading_reset = digitalRead(resetBtn);

// check to see if you just pressed the button
// (i.e. the input went from LOW to HIGH),  and you've waited
// long enough since the last press to ignore any noise:

// If the switch changed, due to noise or pressing:
// resetBtn the debouncing timer

if (reading_start != lastButtonState) {
	lastDebounceTime = millis();
}
// if (reading_reset != lastButtonState) {
// 	lastDebounceTime = millis();
// }


// int fromStart = nowSeconds - startSeconds;
// int countDown = Stage10 - fromStart;


// //duration MINUTES as a whole number
// int totalTimeMinOnly = countDown / 60; // 3

// // duration MINUTES and SECONDS as a decimal
// float totalTimeMin = countDown / (60.00); // 3.3333

// // duration SECONDS as a decimal only
// int totalTimeSec = (( totalTimeMin - totalTimeMinOnly ) * 60);

// int finalTime = (totalTimeMinOnly *100) + totalTimeSec;

if ((millis() - lastDebounceTime) > debounceDelay) {
	// whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
	if (reading_start != buttonState) {
		buttonState = reading_start;
		if (buttonState == HIGH) {
			preWash();

			// kegsWashedSession++;

			// Purge 1


			status_display.writeDigitNum(4, 0); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			startTime = RTC.now();
			startSeconds = (startTime.hour() * 60 * 60) + (startTime.minute() * 60) + startTime.second();

			// currentStage = 0;

			tft.setCursor(5,100);
			tft.print(startTime.hour(), DEC);
			tft.print(':');
			if(startTime.minute() < 10) { //PRINT A 0 IN FRONT OF THE MINUTE IF LESS THAN 10
				tft.print('0');
				tft.print(startTime.minute(), DEC);
				}
			else {
				tft.print(startTime.minute(), DEC);
				}
			tft.print(':');
			if(startTime.second() < 10) {//PRINT A 0 IN FRONT OF THE SECONDS IF LESS THAN 10
				tft.print('0');
				tft.print(startTime.second(), DEC);
				}
			else {
				tft.print(startTime.second(), DEC);
				}
			tft.println(" ");
			}
		}

		tft.setTextColor(ST7735_GREEN,ST7735_BLACK);

		if(nowSeconds == startSeconds + Stage1) {
			tft.setCursor(5,115);


			tft.println("Stage 1");

			// Purge 1
			doPurge();

			status_display.writeDigitNum(4, 1); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage2) {
			tft.setCursor(5,115);
			tft.println("Stage 2");

			// Rinse 1
			doRinse();

			status_display.writeDigitNum(4, 2); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage3) {
			tft.setCursor(5,115);
			tft.println("Stage 3");

			// Purge 2
			doPurge();

			status_display.writeDigitNum(4, 3); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage4) {
			tft.setCursor(5,115);
			tft.println("Stage 4");

			// Rinse 2
			doRinse();

			status_display.writeDigitNum(4, 4); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage5) {
			tft.setCursor(5,115);
			tft.println("Stage 5");

			// Purge 3
			doPurge();

			status_display.writeDigitNum(4, 5); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}


		else if(nowSeconds == startSeconds + Stage6) {
			tft.setCursor(5,115);
			tft.println("Stage 6");

			// Acid Recirc
			doAcidRecirc();

			status_display.writeDigitNum(4, 6); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage7) {
			tft.setCursor(5,115);
			tft.println("Stage 7");

			// Acid purge
			doAcidPurge();

			status_display.writeDigitNum(4, 7); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage8) {
			tft.setCursor(5,115);
			tft.println("Stage 8");

			// final rinse
			doRinse();

			status_display.writeDigitNum(4, 8); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage9) {
			tft.setCursor(5,115);
			tft.println("Stage 9");

			// purge
			doPurge();

			status_display.writeDigitNum(4, 9); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage10) {
			tft.setCursor(5,115);
			tft.println("Stage 10");

			// pause
			doPause();

			status_display.writeDigitNum(3, 1); // Ideally needs to display the cycle number
			status_display.writeDigitNum(4, 0); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

		else if(nowSeconds == startSeconds + Stage11) {
			tft.setCursor(5,115);
			tft.println("Almost Done...");

			// pressure
			doPressure();

			status_display.writeDigitNum(3, 1); // Ideally needs to display the cycle number
			status_display.writeDigitNum(4, 1); // Ideally needs to display the cycle number
			status_display.writeDisplay();


			}

		else if(nowSeconds == startSeconds + Stage12) {
			tft.setCursor(5,115);
			tft.println("Done          ");

			// pressure
			doCo2Pressure();

			digitalWrite(buzzer, 0 );
			delay(1000);
			digitalWrite(buzzer, 1 );

			status_display.writeDigitNum(3, 0); // Ideally needs to display the cycle number
			status_display.writeDigitNum(4, 0); // Ideally needs to display the cycle number
			status_display.writeDisplay();

			}

	}
// save the reading.  Next time through the loop,
// it'll be the lastButtonState:
lastButtonState = reading_start;

}


void doPurge() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, LOW);
	digitalWrite(valve4, HIGH);
	digitalWrite(co2, LOW);
}
void doRinse() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, LOW);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, LOW);
	digitalWrite(valve4, HIGH);
	digitalWrite(co2, HIGH);
}
void doPressure() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, HIGH);
	digitalWrite(valve4, HIGH);
	digitalWrite(co2, LOW);
}
void doCo2Pressure() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, HIGH);
	digitalWrite(valve4, HIGH);
	digitalWrite(co2, HIGH);
}
void doAcidRecirc() {
	digitalWrite(pump, LOW);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, LOW);
	digitalWrite(valve3, HIGH);
	digitalWrite(valve4, LOW);
	digitalWrite(co2, HIGH);
}
void doAcidPurge() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, HIGH);
	digitalWrite(valve4, LOW);
	digitalWrite(co2, LOW);
}
void doPause() {
	digitalWrite(pump, HIGH);
	digitalWrite(valve1, HIGH);
	digitalWrite(valve2, HIGH);
	digitalWrite(valve3, LOW);
	digitalWrite(valve4, HIGH);
	digitalWrite(co2, HIGH);
}

void updateDisplay(byte &stage) {
	tft.setCursor(5,115);
	tft.println("Stage " + stage);

	if(stage <= 0) {
		status_display.writeDigitNum(3, 0);
		status_display.writeDigitNum(4, 0);
	} else if(stage < 10) {
		status_display.writeDigitNum(4, stage);
	} else if(stage < 100) {
		status_display.writeDigitNum(3, stage/10);
		status_display.writeDigitNum(4, stage%10);
	} else {
		//Serial.println("Too big");
	}
	status_display.writeDisplay();
}

void displayTime(DateTime &now) {
	tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);//set text color & size for  DATE coming from TinyRTC
	// tft.setCursor(5,50);
	// tft.print(now.year(), DEC);
	// tft.print('/');
	// tft.print(now.month(), DEC);
	// tft.print('/');
	// tft.print(now.day(), DEC);
	// tft.println(' ');
	tft.setCursor(5,70);
	tft.setTextColor(ST7735_GREEN,ST7735_BLACK); //set color for TIME
	tft.print(now.hour(), DEC);
	tft.print(':');
	if(now.minute() < 10) { //PRINT A 0 IN FRONT OF THE MINUTE IF LESS THAN 10
		tft.print('0');
		tft.print(now.minute(), DEC);
		}
	else {
		tft.print(now.minute(), DEC);
		}
	tft.print(':');
	if(now.second() < 10) {//PRINT A 0 IN FRONT OF THE SECONDS IF LESS THAN 10
		tft.print('0');
		tft.print(now.second(), DEC);
		}
	else {
		tft.print(now.second(), DEC);
		}
	tft.println(" ");
}

void preWash() {
	// loop through the cycles and set their end times
	for (int thisCycle = 0; thisCycle <= cyclesCount-1; thisCycle++) {
		cycles[thisCycle].setEndtime(nowSeconds);
	}
	tft.setCursor(5,60);
	tft.setTextColor(ST7735_CYAN,ST7735_BLACK); //set color for TIME
	tft.println("");
	tft.setCursor(5,80);
	digitalWrite(pump, 1 );
	digitalWrite(valve1, 1 );
	digitalWrite(valve2, 1 );
	digitalWrite(valve3, 0 );
	digitalWrite(valve4, 1 );
	digitalWrite(co2, 1 );
}

void postWash() {
	// reset the vars
}


//-----------------------------------------------------------------
// Bit mask codes. These can be used with writeDigitRaw to
// print various letters and symbols in a digit on the
// seven segment LED display.
// The first argument is the position
// on the 4-digit display, starting at 0 (far left),
// ending at 4 (far right), with position 2 being
// the colon (so you'll skip this position usually).
// The second argument is an integer that corresponds
// to the binary representation of the combination of
// 7 segments making up each digit.
//         0
//      ------
//   5  |     | 1
//      |  6  |
//      ------
//   4  |     | 2
//      |     |
//      ------   o7
//         3
// For example, to make upper case A, you want to
// light up segments 0,1,2,4,5,6.     b00110000
// The binary representation would be b01110111.
//           The bit positions are -->b76543210
// The corresponding integer value is 119 for A.
// Use a binary-to-decimal calculator to convert.
// Below is a list of codes for letters and symbols

//  sevenseg.writeDigitRaw(0,119); // 119 = "A"
//  sevenseg.writeDigitRaw(0,124); // 124 = "b"
//  sevenseg.writeDigitRaw(0,57); // 57 = "C"
//  sevenseg.writeDigitRaw(0,94); // 94 = "d"
//  sevenseg.writeDigitRaw(0,121); // 121 = "E"
//  sevenseg.writeDigitRaw(0,113); // 113  = "F"
//  sevenseg.writeDigitRaw(0,125); // 125 = "G"
//  sevenseg.writeDigitRaw(0,118); // 118 = "H"
//  sevenseg.writeDigitRaw(0,116); // 116 = "h"
//  sevenseg.writeDigitRaw(0,6); // 6 = "I" aka 1
//  sevenseg.writeDigitRaw(0,4); // 4 = "i"
//  sevenseg.writeDigitRaw(0,30); // 30 = "J"
//  sevenseg.writeDigitRaw(0,56); // 56 = "L"
//  sevenseg.writeDigitRaw(0,84); // 84 = "n"
//  sevenseg.writeDigitRaw(0,92); // 92 = "o"
//  sevenseg.writeDigitRaw(0,115); // 115 = "P"
//  sevenseg.writeDigitRaw(0,103); // 103 = "q" aka 9
//  sevenseg.writeDigitRaw(0,80); // 80 = "r"
//  sevenseg.writeDigitRaw(0,109); // 109 = "S" aka 5
//  sevenseg.writeDigitRaw(0,70); // 70 = "t"
//  sevenseg.writeDigitRaw(0,28); // 28 = "u"
//  sevenseg.writeDigitRaw(0,62); // 62 = "U"
//  sevenseg.writeDigitRaw(0,110); // 110 = "y"
//  sevenseg.writeDigitRaw(0,64); // 64 = "-" negative sign
//  sevenseg.writeDigitRaw(0,34); // 34 = " " " double quote
//  sevenseg.writeDigitRaw(0,32); // 32 = "'" apostrophe (upper left)
//  sevenseg.writeDigitRaw(0,2); // 2 = "'" apostrophe (upper right)
//  sevenseg.writeDigitRaw(0,8); // 8 = "_" underscore
//  sevenseg.writeDigitRaw(0,1); // 1 = "-" overbar
//  sevenseg.writeDigitRaw(0,16); // 16 = lower left bar
//  sevenseg.writeDigitRaw(0,4); // 4 = lower right bar
//  sevenseg.writeDigitRaw(0,128); // 128 = "." decimal
//  sevenseg.writeDigitRaw(0,0); // blank
//  sevenseg.writeDigitRaw(2,255); // 255 = ":"  colon, always position 2


//  #define  ST7735_BLACK   0x0000
//  #define  ST7735_BLUE    0x001F
//  #define  ST7735_RED     0xF800
//  #define  ST7735_GREEN   0x07E0
//  #define  ST7735_CYAN    0x07FF
//  #define  ST7735_MAGENTA 0xF81F
//  #define  ST7735_YELLOW  0xFFE0
//  #define  ST7735_WHITE   0xFFFF
//
