#include <Arduino.h>
#include <lx16a-servo.h>
LX16ABus servoBus;
LX16AServo servo(&servoBus, LX16A_BROADCAST_ID); // send these commands to any motor on the bus
int id = 1;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

void setup() {
	servoBus.beginOnePinMode(&Serial2, 14);

	Serial.begin(115200);

	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}

	// Show initial display buffer contents on the screen --
	// the library initializes this with an Adafruit splash screen.
	display.display();
	delay(2000); // Pause for 2 seconds

	// Clear the buffer
	display.clearDisplay();

	// Draw a single pixel in white
	display.drawPixel(10, 10, SSD1306_WHITE);

	// Show the display buffer on the screen. You MUST call display() after
	// drawing commands to make them visible on screen!
	display.display();
	pinMode(16, INPUT_PULLUP);
	pinMode(19, INPUT_PULLUP);
	pinMode(18, INPUT_PULLUP);

}

enum buttonstate {
	UP_PRESSED, DOWN_PRESSED, GO_Pressed, IDLE
} state = IDLE;

void loop() {
	delay(20);
	switch (state) {
	case UP_PRESSED:

		if (digitalRead(16)) {
			state = IDLE;
			Serial.println("Up ");
			id+=1;
		}
		break;
	case DOWN_PRESSED:
		if (digitalRead(19)) {
			state = IDLE;
			Serial.println("down ");
			if(id>1)
				id-=1;
		}
		break;
	case GO_Pressed:
		if (digitalRead(18)) {
			state = IDLE;
			Serial.println("go! ");
			servo.id_write(id);
			Serial.println("Setting to ID " + String(id));
		}
		break;
	case IDLE:
		if (!digitalRead(16)) {
			state = UP_PRESSED;
		}
		if (!digitalRead(19)) {
			state = DOWN_PRESSED;
		}
		if (!digitalRead(18)) {
			state = GO_Pressed;
		}
		break;

	}

	// Set any motor plugged in to ID 3
	// this INO acts as an auto-provisioner for any motor plugged in
	//
	display.clearDisplay();

	display.setTextSize(1);      // Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE); // Draw white text
	display.setCursor(0, 0);     // Start at top-left corner
	display.cp437(true);         // Use full 256 char 'Code Page 437' font

	// Not all the characters will fit on the display. This is normal.
	// Library will draw what it can and the rest will be clipped.
//	for (int16_t i = 0; i < 256; i++) {
//		if (i == '\n')
//			display.write(' ');
//		else
//			display.write(i);
//	}
	int idRead = servo.id_read();
	if(idRead>0)
		display.println("Read " + String(servo.id_read())+"\nSet To "+String(id));
	else
		display.println("Servo not responding");

	display.display();
}

