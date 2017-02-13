
#include <LiquidCrystal.h>
#include <AD9850.h>    //http://github.com/F4GOJ/AD9850

#define MHZ_10  10000000
#define MHZ     1000000
#define KHZ_100 100000
#define KHZ_10  10000
#define KHZ     1000
#define HZ_100  100
#define HZ_10   10

#define MAX_F     40000000
#define MIN_F     500000
#define INITIAL_F 7000000

#define IRQ_FILTER_MS 5
#define CW  1  // Clockwise rotation
#define CCW -1 // Counter Clockwise rotation

#define COMPENSATION 100

// Steps used in the 
double step[5] = {HZ_100,KHZ,KHZ_10,KHZ_100,MHZ};
int currentStep = 0; // Point to the current step
double freq = INITIAL_F; // Frequency in Hz

// Updated by the ISR (Interrupt Service Routine)
int updateScreenFlag = 1;

// Pin connections
const int PinA = 3;   // PinA and PinB for AB encoder
const int PinB = 4;
const int PinSW = 2;  // Push button

// Setup the LCD connections
LiquidCrystal lcd(11, 10, A0, A1, A2, A3);

void displayStep();
void displayFreq();
long getDigit(long mask, long freq);

void setup() {

  // Rotary pulses are INPUTs
  pinMode(PinA, INPUT);
  pinMode(PinB, INPUT);

  // Switch is floating so use the in-built PULLUP so we don't need a resistor
  pinMode(PinSW, INPUT_PULLUP);

  digitalWrite(PinA, HIGH);
  digitalWrite(PinB, HIGH); // Enable the internal pull ups in both input pins

  // Attach the routine to service the interrupts
  attachInterrupt(digitalPinToInterrupt(PinA), isrAB, LOW);
  
  lcd.begin(16, 2);
  DDS.begin(6, 7, 8, 9);
  DDS.calibrate(124999000);
  DDS.setfreq(freq, 0);
}

void loop() {
  
  // Is someone pressing the rotary switch?
  if ((!digitalRead(PinSW))) {
    //virtualPosition = 0;
    while (!digitalRead(PinSW))
      delay(10);
     currentStep = ((currentStep + 1) % 5);
    updateScreenFlag = 1;
  }
  
    DDS.setfreq(freq-COMPENSATION, 0);
    displayFreq();

    if (updateScreenFlag)
    {
      displayStep();
      updateScreenFlag = 0;
    }
}

void displayStep()
{
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Step:");
    lcd.setCursor(8,1);

    switch(currentStep) {
      case 0:
        lcd.print("100  Hz");
        break;
      case 1:
        lcd.print("  1 KHz");
        break;
      case 2:
        lcd.print(" 10 KHz");
        break;
      case 3:
        lcd.print("100 KHz");
        break;
      case 4:
        lcd.print("  1 MHz");
        break;
    }
}

void displayFreq()
{
    lcd.setCursor(0,0);
    lcd.print("Freq:");
    lcd.setCursor(6,0);
    lcd.setCursor(7,0);

    if ( freq >= MHZ_10) 
    {
      lcd.print((int) getDigit(MHZ_10,(long)freq));
    }
    else
    {
      lcd.print(" ");
    }
    
    lcd.setCursor(8,0);
    lcd.print((int)getDigit(MHZ,(long)freq % MHZ_10));
    lcd.setCursor(9,0);
    lcd.print(".");
    lcd.setCursor(10,0);
    lcd.print((int)getDigit(KHZ_100,(long)freq % MHZ));
    lcd.setCursor(11,0);
    lcd.print((int)getDigit(KHZ_10,(long)freq % KHZ_100));
    lcd.setCursor(12,0);
    lcd.print((int)getDigit(KHZ,(long)freq % KHZ_10));
    lcd.setCursor(13,0);
    lcd.print(",");
    lcd.setCursor(14,0);
    lcd.print((int)getDigit(HZ_100,(long)freq % KHZ));
    lcd.setCursor(15,0);
    lcd.print((int)getDigit(HZ_10,(long)freq % HZ_100));
}

long getDigit(long mask, long freq)
{
    return (freq/mask);
}


// https://github.com/futureshocked/arduino_sbs/blob/master/RotaryEncoder/rotaryEncoder/rotaryEncoder.ino
void isrAB ()  {
  
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  char direction = 0;
  
  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > IRQ_FILTER_MS) {
    if (digitalRead(PinB) == LOW)
    {
      direction = CCW ;
    }
    else {
      direction = CW ;
    }

    if (freq >=MAX_F & direction == CW)
      freq = MAX_F;
    if (freq <= MIN_F & direction == CCW)
      freq = MIN_F;

    if ((freq >= MAX_F & direction == CCW) | 
      (freq <= MIN_F & direction == CW) |
      (freq > MIN_F & freq < MAX_F)
      )
      freq = freq + (direction * step[currentStep]);
      lastInterruptTime = interruptTime;
  }
}

