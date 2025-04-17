/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-rotary-encoder-led
 */

#include <Servo.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_ST7565_ZOLEN_128X64_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 7, /* data=*/ 4, /* cs=*/ 2, /* dc=*/ 3, /* reset=*/ 8);  // Pro Mini pinout
//#define U8LOG_WIDTH 21 // 20 caracteres no original, display da point mini cabe 21 caracteres
//#define U8LOG_HEIGHT 8
//uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];
//U8G2LOG u8g2log;

#define CLK_PIN 6
#define DT_PIN 5
#define LED_PIN 13

#define DIRECTION_CW 0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

int counter = 0;
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;
int brightness = 125;  // middle value

void setup() {

  Serial.begin(115200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  //u8g2.setFont(u8g2_font_6x10_tf);
  //u8g2.setFontRefHeightExtendedText();
  //u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  //u8g2log.setLineHeightOffset(0);	// set extra space between lines in pixel, this can be negative
  //u8g2log.setRedrawMode(0);		// 0: Update screen with newline, 1: Update screen for every char 

  // configure encoder pins as inputs
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);

  // read the initial state of the rotary encoder's CLK pin
  prev_CLK_state = digitalRead(CLK_PIN);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  // read the current state of the rotary encoder's CLK pin
  CLK_state = digitalRead(CLK_PIN);

  // If the state of CLK is changed, then pulse occurred
  // React to only the rising edge (from LOW to HIGH) to avoid double count
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    // if the DT state is HIGH
    // the encoder is rotating in counter-clockwise direction => decrease the counter
    if (digitalRead(DT_PIN) == HIGH) {
      direction = DIRECTION_CCW;
      counter--;
      brightness -= 10;  // you can change this value
    } else {
      // the encoder is rotating in clockwise direction => increase the counter
      direction = DIRECTION_CW;
      counter++;
      brightness += 10;  // you can change this value
    }

    if (brightness < 0)
      brightness = 0;
    else if (brightness > 255)
      brightness = 255;

    // sets the brightness of LED according to the counter
    analogWrite(LED_PIN, brightness);
    Serial.println(brightness); // to Leonardo

  }

  // save last CLK state
  prev_CLK_state = CLK_state;

  //Serial.println(brightness); // to Leonardo
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setCursor(0,8);
  u8g2.print(brightness);	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
}
