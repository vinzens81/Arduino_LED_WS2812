#include <Adafruit_NeoPixel.h>
#include <PinChangeInt.h>

#define PIN 7
#define NUMPIXELS 200
#define BUTTON 8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const int HueNumReadings = 10;
int HueReadings[HueNumReadings];
int HueReadIndex = 0;
int HueTotal = 0;
int HueAverage = 0;
int OldHueAverage = 0;
int OldHueAverageDiviation = 0;
int HueInputPin = A0;

const int LightNumReadings = 10;
int LightReadings[LightNumReadings];
int LightReadIndex = 0;
int LightTotal = 0;
int LightAverage = 0;
int OldLightAverage = 0;
int OldLightAverageDiviation = 0;
int LightInputPin = A1;

int RGBArray[3];

int minLight = 0;
int maxLight = 255;
int idleRun = 0;

boolean run = false;
boolean buttonPressed = false;

String rgbValue = "FFFFFF";

void setup() {
  Serial.begin(9600);
  Serial.println("Hello!");
  for (int thisReading = 0; thisReading < HueNumReadings; thisReading++) {
    HueReadings[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < LightNumReadings; thisReading++) {
    LightReadings[thisReading] = 0;
  }
  pixels.begin();

  PCintPort::attachInterrupt(BUTTON, buttonPress,CHANGE);
  for(int i=0;i<NUMPIXELS;i++) {
    pixels.setPixelColor(i,0,0,0);
  }
  pixels.show();

}

void loop() {
  if (run) {
    HueTotal = HueTotal - HueReadings[HueReadIndex];
    HueReadings[HueReadIndex] = analogRead(HueInputPin);
    HueTotal = HueTotal + HueReadings[HueReadIndex];
    HueReadIndex = HueReadIndex + 1;
    if (HueReadIndex >= HueNumReadings) {
      HueReadIndex = 0;
    }
    OldHueAverage = HueAverage;
    HueAverage = HueTotal / HueNumReadings;
    OldHueAverageDiviation=OldHueAverage-HueAverage;
    if (OldHueAverageDiviation<0){
      OldHueAverageDiviation=OldHueAverageDiviation*-1;
    }

    LightTotal = LightTotal - LightReadings[LightReadIndex];
    LightReadings[LightReadIndex] = analogRead(LightInputPin);
    LightTotal = LightTotal + LightReadings[LightReadIndex];
    LightReadIndex = LightReadIndex + 1;
    if (LightReadIndex >= LightNumReadings) {
      LightReadIndex = 0;
    }
    OldLightAverage = LightAverage;
    LightAverage = LightTotal / LightNumReadings;
    OldLightAverageDiviation = OldLightAverage-LightAverage;
    if (OldLightAverageDiviation<0){
      OldLightAverageDiviation=OldLightAverageDiviation*-1;
    }

    getMinMaxLight(LightAverage);
    getRGB(HueAverage);
    if (OldLightAverageDiviation>4 or OldHueAverageDiviation>4 or idleRun>100) {
      for(int i=0;i<NUMPIXELS;i++) {
        pixels.setPixelColor(i, pixels.Color(RGBArray[0],RGBArray[1],RGBArray[2]));
      }
      Serial.println("Changing Pixels.......");
      pixels.show();
      idleRun=0;
    } 
    else {
      Serial.println("Doing Nothing.. sleeping longer");
      delay(100);
    }
    idleRun++;
    delay(100);
  }
  else
  {
    delay(500);
    idleRun++;
    if (idleRun > 1000) { // set LED to off from time to time because of failure in the LED's
      idleRun=0;
      for(int i=0;i<NUMPIXELS;i++) {
        pixels.setPixelColor(i,0,0,0); // all off
      }
      pixels.show();
    }

  }
}

void getMinMaxLight(int light) {
  maxLight = 255;
  minLight = 0;
  switch(light) {
  case 0 ... 512:
    {
      maxLight = (light/512.0*255.0);
    }
    break;
  case 513 ... 1023:
    {
      minLight = (light-512)/512.0*255.0;
    }
    break;
  }
}
int getColorValueRising(int hueAngle, int angleDiff) {
  float range=maxLight-minLight;
  int color=0;
  color=((255.0/60.0*(hueAngle-angleDiff))/255.0*range)+minLight;
  if (color > 255) {
    color=255;
  }
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
  case 0 ... 60:
    {
      RGBArray[0]=maxLight;
      RGBArray[1]=getColorValueRising(hueAngle, 0);
    }
    break;
  case 61 ... 120:
    {
      RGBArray[0]=getColorValueFalling(hueAngle, 60);
      RGBArray[1]=maxLight;
    }
    break;
  case 121 ... 180:
    {
      RGBArray[1]=maxLight;
      RGBArray[2]=getColorValueRising(hueAngle, 120);
    }
    break;
  case 181 ... 240:
    {
      RGBArray[1]=getColorValueFalling(hueAngle, 180);
      RGBArray[2]=maxLight;
    }
    break;
  case 241 ... 300:
    {
      RGBArray[0]=getColorValueRising(hueAngle, 240);
      RGBArray[2]=maxLight;
    }
    break;
  case 301 ... 355: // last 5° for white only. Here min value is set.
    {
      RGBArray[0]=maxLight;
      RGBArray[2]=getColorValueFalling(hueAngle, 300);
    }
    break;
  }
  return;
}

void buttonPress() {
  if (not buttonPressed) {
    buttonPressed=true; // avoid double pressing the buttun while doing stuff
    if(digitalRead(BUTTON) == HIGH) {
      if (run) {
        run=false;
        for(int i=0;i<NUMPIXELS;i++) {
          pixels.setPixelColor(i,0,0,0);
        }
        pixels.show();
        delay(1000);
        for(int i=0;i<NUMPIXELS;i++) {
          pixels.setPixelColor(i,0,0,0);
        }
        pixels.show();

      }
      else {
        run=true;
        delay(1000);
      }
    }
    buttonPressed=false;
  }
}




