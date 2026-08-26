//Arduino Signal generator 1Hz-1MHz, PWM 0%-100%, minimum on/off time 0.2µs
//Arduino Nano/Uno D9 is output, use Serial monitor at 9600 to enter freq/duty%
#include <TimerOne.h>
unsigned long t = 1000; // default 1000 μs (1000 Hz)
unsigned long k = 512;  // default duty cycle 50%
unsigned long f;
unsigned long dutyCycle = 50;
const unsigned long maxFrequency = 1000000; // 1 MHz
const unsigned long minFrequency = 1; // 1 Hz
const unsigned long maxDutyCycle = 100; // 100%
const unsigned long minDutyCycle = 0; // 0%
const float MIN_ON_TIME = 0.2; // 0.2 us

void setup() {
  delay(200);//prevent serial to print garbage
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  setPWM();
  Serial.println("Signal generator 1Hz-1MHz, 0-100% dutycycle, min Ton/Toff 0.2µs");
}

void loop() {
  readFrequency ();   // Read the frequency input from the serial monitor
  readDutycycle ();   // Read the duty cycle input from the serial monitor
  checkOnOfftime(); //check if on/off time is within limits
  setPWM();
}

void readFrequency () {
  Serial.print("Enter Frequency 1 to 1000000 [Hz]: ");
  while (Serial.available() == 0) ; //stay in infinite while loop until something is entered in the serial monitor
  String input = Serial.readStringUntil('\n');
  Serial.println(  input + " Hz ");
  f = input.toInt();
  if (f >= minFrequency && f <= maxFrequency) {
    t = 1000000 / f; // Calculate period in microseconds
  }
  else {
    Serial.println("Frequency is out of range! Please try again ");
    readFrequency ();//function calls itself, bad coding ;-)
  }
}

void readDutycycle () {
  Serial.print("Enter Dutycycle 0 to 100     [% ]: ");
  while (Serial.available() == 0) ; //stay in infinite while loop until something is entered in the serial monitor
  String input = Serial.readStringUntil('\n');
  Serial.println(  input + " % ");
  dutyCycle = input.toInt();
  if (dutyCycle >= minDutyCycle && dutyCycle <= maxDutyCycle ) {
    k = (dutyCycle * 1024) / 100; // Calculate duty cycle for Timer1.pwm
  }
  else {
    Serial.println("Dutycycle is out of range! Please try again ");
    readDutycycle ();//function calls itself, bad coding ;-)
  }
}

void checkOnOfftime() {
  float onTimeMicros = 1000000.0f * ((dutyCycle / 100.0f) / f);
  float offTimeMicros = 1000000.0f * ((1 - (dutyCycle / 100.0f)) / f);

  if (onTimeMicros >= MIN_ON_TIME  && offTimeMicros >= MIN_ON_TIME || offTimeMicros == 0 || onTimeMicros == 0) {
    Serial.println("Output updated to:  ");
    Serial.print("Frequency: ");
    Serial.print(f);
    Serial.print("Hz, Dutycycle: ");
    Serial.print(dutyCycle);
    Serial.print("%, Ton: ");
    Serial.print(onTimeMicros);
    Serial.print("µs, Toff: ");
    Serial.print(offTimeMicros);
    Serial.println("µs");
    setPWM();
  }
  else {
    Serial.println("On/Off-Time too short. Please try again");

  }
}

void   setPWM() {
  Timer1.initialize(t);
  Timer1.pwm(9, k);
}