#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

#define PIN 7
#define NUMPIXELS 2

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const int HueNumReadings = 10;
int HueReadings[HueNumReadings];
int HueReadIndex = 0;
int HueTotal = 0;
int HueAverage = 0;
int HueInputPin = A0;

const int LightNumReadings = 10;
int LightReadings[LightNumReadings];
int LightReadIndex = 0;
int LightTotal = 0;
int LightAverage = 0;
int LightInputPin = A1;

int RGBArray[3];

int minLight = 0;
int maxLight = 255;

String rgbValue = "FFFFFF";


void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
 
  Serial.println("multiChannels");
  lcd.setCursor(0,0); //Start at character 4 on line 0
  lcd.print("Hello, world!");
  delay(1000);
  for (int thisReading = 0; thisReading < HueNumReadings; thisReading++) {
    HueReadings[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < LightNumReadings; thisReading++) {
    LightReadings[thisReading] = 0;
  }

  pixels.begin();

}

void loop() {

  HueTotal = HueTotal - HueReadings[HueReadIndex];
  HueReadings[HueReadIndex] = analogRead(HueInputPin);
  HueTotal = HueTotal + HueReadings[HueReadIndex];
  HueReadIndex = HueReadIndex + 1;
  if (HueReadIndex >= HueNumReadings) {
    HueReadIndex = 0;
  }
  HueAverage = HueTotal / HueNumReadings;
 
  LightTotal = LightTotal - LightReadings[LightReadIndex];
  LightReadings[LightReadIndex] = analogRead(LightInputPin);
  LightTotal = LightTotal + LightReadings[LightReadIndex];
  LightReadIndex = LightReadIndex + 1;
  if (LightReadIndex >= LightNumReadings) {
    LightReadIndex = 0;
  }
  LightAverage = LightTotal / LightNumReadings;
 
  getMinMaxLight(LightAverage);
 
  getRGB(HueAverage);  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("max:");
  lcd.print(maxLight);
  lcd.setCursor(9,0);
  lcd.print("min:");
  lcd.print(minLight);
 
  lcd.setCursor(0,1);
  lcd.print("r");
  lcd.print(RGBArray[0]);
  lcd.setCursor(5,1);
  lcd.print("g");
  lcd.print(RGBArray[1]);
  lcd.setCursor(10,1);
  lcd.print("b");
  lcd.print(RGBArray[2]);
  for(int i=0;i<NUMPIXELS;i++) {
    pixels.setPixelColor(i, pixels.Color(RGBArray[0],RGBArray[1],RGBArray[2]));
  }
  pixels.show();
  delay(100);
}

void getMinMaxLight(int light) {
  maxLight = 255;
  minLight = 0;
  switch(light) {
    case 0 ... 512: {
      maxLight = (light/512.0*255.0);
    }
    break;
    case 513 ... 1023: {
      minLight = (light-512)/512.0*255.0;
    }
    break;
  }
}
int getColorValueRising(int hueAngle, int angleDiff) {
  float range=maxLight-minLight;
  int color=0;
  color=((255.0/60.0*(hueAngle-angleDiff))/255.0*range)+minLight;
  return (color);
}
int getColorValueFalling(int hueAngle, int angleDiff) {
  float range=maxLight-minLight;
  int color=0;
  color=((255-(255.0/60.0*(hueAngle-angleDiff)))/255.0*range)+minLight;
  if (color > 255) {
    color=255;
  }
  
  return (color);
}

void getRGB(int hue) {
  int hueAngle = hue/1023.0*360.0;
  RGBArray[0] = minLight;
  RGBArray[1] = minLight;
  RGBArray[2] = minLight; 
  switch(hueAngle) {
    case 0 ... 60: {
      RGBArray[0]=maxLight;
      RGBArray[1]=getColorValueRising(hueAngle, 0);
    }
    break;
    case 61 ... 120: {
      RGBArray[0]=getColorValueFalling(hueAngle, 60);
      RGBArray[1]=maxLight;
    }
    break;
    case 121 ... 180: {
      RGBArray[1]=maxLight;
      RGBArray[2]=getColorValueRising(hueAngle, 120);
    }
    break;
    case 181 ... 240: {
      RGBArray[1]=getColorValueFalling(hueAngle, 180);
      RGBArray[2]=maxLight;
    }
    break;
    case 241 ... 300: {
      RGBArray[0]=getColorValueRising(hueAngle, 240);
      RGBArray[2]=maxLight;
    }
    break;
    case 301 ... 360: {
      RGBArray[0]=maxLight;
      RGBArray[2]=getColorValueFalling(hueAngle, 300);
    }
    break;
  }
  return;
}
