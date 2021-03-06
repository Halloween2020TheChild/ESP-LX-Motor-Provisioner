#include <Arduino.h>
#include <lx16a-servo.h>
LX16ABus servoBus;
LX16AServo broadcast(&servoBus, LX16A_BROADCAST_ID); // send these commands to any motor on the bus
int id = 1;
LX16AServo *motor = NULL;
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32AnalogRead.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
ESP32AnalogRead pot;
bool motorPresent = false;
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	if (x < in_min)
		return out_min;
	if (x > in_max)
		return out_max;

	float divisor = (in_max - in_min);
	if (divisor == 0) {
		return -1; //AVR returns -1, SAM returns 0
	}
	return (x - in_min) * (out_max - out_min) / divisor + out_min;
}
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

	display.setTextSize(1);      // Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE); // Draw white text
	display.setCursor(0, 0);     // Start at top-left corner
	display.cp437(true);         // Use full 256 char 'Code Page 437' font

	// Show the display buffer on the screen. You MUST call display() after
	// drawing commands to make them visible on screen!
	display.display();
	pinMode(16, INPUT_PULLUP);
	pinMode(19, INPUT_PULLUP);
	pinMode(18, INPUT_PULLUP);
	pot.attach(A1);
	id = broadcast.id_read();
	if (!broadcast.isCommandOk())
		id = 1;
	else {
		motorPresent = true;
		motor = new LX16AServo(&servoBus, id);
	}
}

enum buttonstate {
	UP_PRESSED, DOWN_PRESSED, GO_Pressed, IDLE
} state = IDLE;

void loop() {
	long time = millis();
	delay(20);
	uint32_t readval = analogRead(A1);
	//Serial.println("Pot at: "+String(readval));
	float potval = mapf(readval, 0, 4095, 0, 24000);
	switch (state) {
	case UP_PRESSED:

		if (digitalRead(16)) {
			state = IDLE;
			Serial.println("Up ");
			id += 1;
		}
		break;
	case DOWN_PRESSED:
		if (digitalRead(19)) {
			state = IDLE;
			Serial.println("down ");
			if (id > 1)
				id -= 1;
		}
		break;
	case GO_Pressed:
		if (digitalRead(18) && id > 0) {
			state = IDLE;
			Serial.println("go! ");
			broadcast.id_write(id);
			if(motor!=NULL)	delete (motor);
			motor = new LX16AServo(&servoBus, id);
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

	display.clearDisplay();
	display.setCursor(0, 0);     // Start at top-left corner

	int idRead = broadcast.id_read();
	if (broadcast.isCommandOk()) {
		if (motor==NULL) {
			motor = new LX16AServo(&servoBus, idRead);
		}
		motorPresent = true;
		display.println(
				"Read  " + String(idRead) + "\nSet To " + String(id)
						+ "\nPos   " + String((potval / 100.0)));

		motor->move_time(potval, millis() - time + 2);
	} else {
		display.println("Servo not responding");
		motorPresent = false;
		if(motor!=NULL)
			delete (motor);
		motor=NULL;
	}

	display.display();
}

