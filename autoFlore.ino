// 
#include <GyverOLED.h>
#define EB_FAST_TIME 100
#include <EncButton.h>
#include <EEPROM.h>
#include "FastLED.h"
#define INIT_ADDR 64
#define INIT_KEY 51

// define pins
#define LED_PIN 7
#define LED_NUM 1
#define pinPompa 6
#define pinWet 0
EncButton eb(2, 3, 4);
GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;
CRGB leds[LED_NUM];
//define varibls
bool oledPower = true;
int oledTimout = 30;

uint32_t secOld = 0;
uint32_t MinsOld = 0;


//wet
int wet_val = 0;
int wet_count = 3;
int wet_current = 0;

//pompa
int timeout_var = 720;
int deltaPompa_var = 30;
int stopPompa_var = 0;
int stop_timeout_var = timeout_var;


// colors
//Green;Orange;OrangeRed;Red;
int green = 240;
int orange = 270;
int orangered = 300;
int red = 330;
bool alarm = false;

//menu
class Menu {
  public:
    int page = 0;
    bool timeout = false;
    bool deltaPompa = false;
    void show_timeout() {
      oled.clear();
      oled.home();
      oled.setScale(3);
      oled.print(timeout_var);
      oled.update();
    }
    void show_deltaPompa() {
      oled.clear();
      oled.home();
      oled.setScale(3);
      oled.print(deltaPompa_var);
      oled.update();
    }
    void show() {
      oled.clear();  // очистка
      timeout = false;
      deltaPompa = false;
      oled.home();
      oled.setScale(2);
      switch (page) {
        case 0:
          oled.print("Menu 0");
          oled.setCursor(10, 2);
          oled.print(stop_timeout_var / 60ul);
          oled.print("h ");
          oled.print(stop_timeout_var % 60ul);
          oled.print("m ");
          break;
        case 1:
          oled.print("Menu 1");
          oled.setCursor(30, 2);
          oled.print("wet: ");
          oled.print(wet_val);
          break;
        case 2:
          oled.print("Menu 2");
          oled.setCursor(0, 2);
          oled.print("every:");
          oled.print(timeout_var);
          oled.print("m");
          break;
        case 3:
          oled.print("Menu 3");
          oled.setCursor(0, 2);
          oled.print("during:");
          oled.print(deltaPompa_var);
          oled.print("s");
          break;
        default:
          oled.print("Menu 777");
          break;
      }
      oled.update();
    }
    void next(int r){
      page += r;
      if (page < 0) {page = 3;}
      if (page > 3) {page = 0;}
    }        // Attribute (int variable)
};
Menu m;

//
void cb() {
  switch (eb.action()) {
    case EB_PRESS:
      break;
    case EB_HOLD:
      oledTimout = 30;
 //     if (oledPower == true) {oled.setPower(false); oledPower = false;}
 //     digitalWrite(pinPompa,LOW);
      if (oledPower == false) {
        oled.setPower(true); 
        oledPower = true; 
        m.page = 0;
        m.show();
        }
      else if (m.timeout == true) {
        EEPROM.put(0, timeout_var);
        stop_timeout_var = timeout_var;
        m.show();
      }
      else if (m.deltaPompa == true) {
        EEPROM.put(2, deltaPompa_var);
        m.show();
      }
      else if (m.page == 0){
        stopPompa_var = deltaPompa_var;
        stop_timeout_var = timeout_var;
      }
      break;
    case EB_STEP:
      break;
    case EB_RELEASE:
      break;
    case EB_CLICK:
      oledTimout = 30;
      //oled.setPower(true);
      //stopPompa = (millis()  / 1000ul) + deltaPompa;
      //digitalWrite(pinPompa,HIGH);
      if (oledPower == false) {
        oled.setPower(true); 
        oledPower = true; 
        m.page = 0;
        m.show();
        }
      else if (m.timeout == 0 & m.deltaPompa ==0){
        if (m.page == 2) {
          m.timeout = true;
          m.show_timeout();
        }
        else if (m.page == 3) {
          m.deltaPompa = true;
          m.show_deltaPompa();
        }
        else if (m.page == 1) {
          m.show();
        }    
      }
      else if (m.timeout == true) {
        m.page == 2;
        EEPROM.get(0, timeout_var);
        m.show();
      }
      else if (m.deltaPompa == true) {
        m.page == 3;
        EEPROM.get(2, deltaPompa_var);
        m.show();
      }
      break;
    case EB_CLICKS:
      //Serial.println(eb.getClicks());
      break;
    case EB_TURN:
      oledTimout = 30;
      if (oledPower == false) {
        oled.setPower(true); 
        oledPower = true; 
        m.page = 0;
        m.show();
        }
      else if (m.timeout == true){
        timeout_var += -eb.dir() * (1 + 25 * eb.fast());
        m.show_timeout();
        }
      else if (m.deltaPompa == true){
        deltaPompa_var += -eb.dir() * (1 + 10 * eb.fast());
        m.show_deltaPompa();
      }
      else {
          m.next(-eb.dir());
          m.show();
        } 
      break;
    case EB_REL_HOLD:
      break;
    case EB_REL_HOLD_C:
      break;
    case EB_REL_STEP:
      break;
    case EB_REL_STEP_C:
      break;
    default:
     m.show();
  }
}
void setup() {
  FastLED.addLeds< WS2811, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.setBrightness(25);
  leds[0] = CRGB::Green;
  FastLED.show();
  oled.init();   // инициализация
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // первый запуск
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, timeout_var);
    EEPROM.put(2, deltaPompa_var);
  }
  else{
    EEPROM.get(0, timeout_var);
    EEPROM.get(2, deltaPompa_var);
    stop_timeout_var = timeout_var;
  }
  pinMode(pinPompa, OUTPUT);
  eb.attach(cb);
  eb.setEncType(EB_STEP2);
  m.show();

  //define local varible
}
void loop() {
  eb.tick();
  uint32_t sec = millis() / 1000ul;      // полное количество секунд
//  int timeHours = (sec / 3600ul);        // часы
  int Mins = sec / 60ul;  // полное количество минут
//  int Mins = (sec % 3600ul) / 60ul;  // минуты
//  int timeSecs = (sec % 3600ul) % 60ul;  // секунды
  if (sec != secOld) {
    secOld = sec;

    // wet

    wet_current += analogRead(pinWet);
    wet_count -= 1;
    if (wet_count == 0) {
      wet_val = wet_current / 3;
      wet_current = 0;
      wet_count = 3;
    }

    // wet led

    if (wet_val < green) {
      FastLED.setBrightness(25);
      leds[0] = CRGB::Green;
      FastLED.show();
    }
    else if (wet_val < orange) {
      FastLED.setBrightness(25);
      leds[0] = CRGB::Orange;
      FastLED.show();
    }
    else if (wet_val < orangered) {
      FastLED.setBrightness(25);
      leds[0] = CRGB::OrangeRed;
      FastLED.show();
    }
    else if (wet_val < red) {
      FastLED.setBrightness(50);
      leds[0] = CRGB::Red;
      FastLED.show();
    }
    else {
      if (alarm == false) {
      FastLED.setBrightness(150);
      leds[0] = CRGB::Red;
      FastLED.show();
      alarm = true;
      }
      else {
      FastLED.setBrightness(1);
      leds[0] = CRGB::Black;
      FastLED.show();
      alarm = false;       
      }
    }

  // oled timeout

    if (oledTimout > 0) { oledTimout -= 1;}
    else {
      oledPower = false;
      oled.setPower(false);
    }
  
  // pompa

    if (stopPompa_var > 0) {
      digitalWrite(pinPompa,HIGH);
      stopPompa_var -= 1;
      stop_timeout_var = timeout_var;
      }
    else {
      digitalWrite(pinPompa,LOW);
    }


  }

  // pompa
  if (Mins != MinsOld) {
    MinsOld = Mins;
    if (stop_timeout_var > 0) {
      stop_timeout_var -= 1;
    }
    else {
      stopPompa_var = deltaPompa_var;

//      oledPower = false;
//      oled.setPower(false);
    }
  }
/*
  if (sec != secOld) {
    secOld = sec;
    oled.clear();
    oled.home();
    oled.setScale(3);
    oled.print(timeHours);oled.print(":");oled.print(timeMins);oled.print(":");oled.print(timeSecs);
    oled.update();
  }
  if (stopPompa > sec){
    digitalWrite(pinPompa,HIGH);
  }
  else {
    digitalWrite(pinPompa,LOW);
  }
*/
}
