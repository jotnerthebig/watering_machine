/*
 * Watering Machine, version 1.0
 * Features:
 * - soil humidity measurement;
 * - display of current and minimum humidity levels;
 * - setting the minimum humidity level using the "up" and "down" buttons;
 * - display of the mode symbol ("watering" or "LED");
 * - enabling watering/indication when the minimum humidity level is reached;
 * Protection purposes:
 * - turning on watering for 1 second (not changed by the user in this version);
 * - 10 seconds delay after each watering (not changed by the user in this version);
 * - the maximum value of the minimum humidity level is set to 95%.
 */

#include <U8g2lib.h>

#define ONBOARD_LED       13
#define RED_LED_PIN       8

#define PUMP_PIN          3
#define SENSOR_PIN        A0

#define JOYSTICK_MID      4
#define JOYSTICK_UP       7
#define JOYSTICK_DOWN     5
#define JOYSTICK_LEFT     2
#define JOYSTICK_RIGHT    6

#define MIN_HUMIDITY      280
#define MAX_HUMIDITY      670

#define ALARM_DEFAULT     50
#define CHANGE_STEP       5

#define DISPLAY_FULL      0
#define DISPLAY_MODE      1
#define DISPLAY_ALARM     2

#define MODE_WATER        true
#define MODE_ALARM        false

#define MODE_GLYPH_FONT   u8g2_font_unifont_t_77
#define MODE_SYMBOL_WATER 0x26c8
#define MODE_SYMBOL_ALARM 0x2699

#define WATERING_DURATION 2000
#define OVERWATER_DELAY   10000

#define ABS_MAX_HUMIDITY  95
#define ABS_MIN_HUMIDITY  0

constexpr uint8_t humidity_sensor_pin = SENSOR_PIN;
volatile int humidity_sensor_value    = 0;

long humidity_percent = 0;
long humidity_alarm   = ALARM_DEFAULT;

bool mode_alarm      = MODE_ALARM;
uint16_t mode_symbol = MODE_SYMBOL_ALARM;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

void display_text(u8g2_uint_t x, u8g2_uint_t y, const uint8_t *font, const char *text) {
  u8g2.setFont(font);
  u8g2.drawStr(x, y, text);
}

void display_current() {
  const char cur_message[] = "cur:";
  display_text(18, 7, u8g2_font_simple1_tf, cur_message);
  char value_str[12] = "";
  (humidity_percent >= 0) ? ltoa(humidity_percent, value_str, 10) : NULL;
  display_text(13, 30, u8g2_font_callite24_tr, value_str);
}

void display_mode() {
  (mode_alarm == MODE_ALARM) ? mode_symbol = MODE_SYMBOL_ALARM : mode_symbol = MODE_SYMBOL_WATER;
  u8g2.setFont(MODE_GLYPH_FONT);
  u8g2.drawGlyph(52, 23, mode_symbol);
}

void display_alarm() {
  const char min_txt[] = "min:";
  display_text(85, 7, u8g2_font_simple1_tf, min_txt);
  char value_alarm[7] = "";
  (humidity_alarm >= 0) ? ltoa(humidity_alarm, value_alarm, 10) : NULL;
  display_text(82, 30, u8g2_font_logisoso16_tf, value_alarm);
}

/*
  Display dashboard:
  0 - full display (current humidity level, mode symbol, alarm humidity level)
  1 - mode symbol only
  2 - alarm humidity only
*/
void display_dashboard(uint8_t mode) {
  u8g2.clearBuffer();
  switch (mode) {
    case DISPLAY_FULL: {
      display_current();
      display_mode();
      display_alarm();
      break;
    }
    case DISPLAY_MODE: {
      display_mode();      
      break;
    }
    case DISPLAY_ALARM: {
      display_alarm();
      break;
    }
  }
  u8g2.sendBuffer();
}

void get_humidity_value() {
  humidity_sensor_value = analogRead(humidity_sensor_pin);
  humidity_percent = map((long)humidity_sensor_value, MAX_HUMIDITY, MIN_HUMIDITY, 0, 100);
}

void LED_blink(unsigned int pause) {
  digitalWrite(RED_LED_PIN, HIGH);
  delay(pause);
  digitalWrite(RED_LED_PIN, LOW);
  delay(pause);
}

void pump_on(unsigned long msec) {
  digitalWrite(PUMP_PIN, HIGH);
  delay(msec);
  digitalWrite(PUMP_PIN, LOW);
}

void setup() {
  /* LED */
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  /* Pump */
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  /* Joystick */
  pinMode(JOYSTICK_MID, INPUT_PULLUP);
  pinMode(JOYSTICK_UP, INPUT_PULLUP);
  pinMode(JOYSTICK_DOWN, INPUT_PULLUP);
  pinMode(JOYSTICK_LEFT, INPUT_PULLUP);
  pinMode(JOYSTICK_RIGHT, INPUT_PULLUP);
  /* Display */
  u8g2.begin();
}

/* Main loop */
void loop() {
  get_humidity_value();
  display_dashboard(DISPLAY_FULL);
  if(humidity_percent <= humidity_alarm) {
    switch (mode_alarm) {
      case MODE_ALARM: {
        LED_blink(100);
        break;
      }
      case MODE_WATER: {
        pump_on(WATERING_DURATION);
        delay(OVERWATER_DELAY); // STUPID DELAY
        break;
      }
    }
  }
  if (digitalRead(JOYSTICK_UP) == LOW) {
    if (humidity_alarm <= ABS_MAX_HUMIDITY) {
      humidity_alarm += CHANGE_STEP;
      delay(100);
    }
  }
  if (digitalRead(JOYSTICK_DOWN) == LOW) {
    if (humidity_alarm >= ABS_MIN_HUMIDITY) {
      humidity_alarm -= CHANGE_STEP;
      delay(100);
    }
  }
  if (digitalRead(JOYSTICK_LEFT) == LOW) {
    mode_alarm = MODE_ALARM;
    delay(100);
  }
  if (digitalRead(JOYSTICK_RIGHT) == LOW) {
    mode_alarm = MODE_WATER;
    delay(100);
  }
}
