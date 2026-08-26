/*
ESP32 PWM Generator - 1Hz - 40Mhz
PWM from GPIO18
Usage: Enter frequency (f1000) or dutycycle (45%) in Serial Monitor @ 115200 baud.
Max frequency: 40MHz (with 1-bit resolution)
*/

#include <driver/ledc.h>

// PWM Configuration
int PWM_FREQ = 1000;               // Default 1kHz
int PWM_RESOLUTION = 10;           // Use 10-bit resolution (0-1023)
int LED_OUTPUT_PIN = 18;           // GPIO pin for PWM output
ledc_channel_t PWM_CHANNEL = LEDC_CHANNEL_0;  // Use channel 0
ledc_timer_t PWM_TIMER = LEDC_TIMER_0;        // Use timer 0

// Constants
const int MAX_FREQ = 40000000;     // 40MHz maximum 
const int MIN_FREQ = 1;            // 1Hz minimum

// Global variables
int max_duty_value = (1 << PWM_RESOLUTION) - 1;
float current_duty_input_percent = 50.0; 
float current_duty_hardware_percent = 50.0; 

String inputString = "";
bool stringComplete = false;

void printManual() {
  Serial.println("=== ESP32 High Frequency PWM Generator ===");
  Serial.println("Frequency range: 1Hz to 40MHz");
  Serial.println("");
  Serial.println("COMMANDS:");
  Serial.println("  f1000      - Set frequency to 1kHz");
  Serial.println("  f10M       - Set frequency to 10MHz");
  Serial.println("  f20M       - Set frequency to 20MHz");
  Serial.println("  f40M       - Set frequency to 40MHz (square wave)");
  Serial.println("  45%        - Set duty cycle to 45%");
  Serial.println("");
  Serial.println("NOTES:");
  Serial.println("  • Resolution auto-adjusts (1-10 bits)");
  Serial.println("  • >10MHz: Use 10:1 oscilloscope probe");
  Serial.println("  • >20MHz: Very short wires recommended");
  Serial.println("==========================================");
}

// Calculate optimal resolution for given frequency
int calculateResolution(long frequency) {
  if (frequency >= 20000000) {     // 20MHz - 40MHz: 1-bit (square wave)
    return 1;
  } else if (frequency >= 10000000) { // 10MHz - 20MHz: 2-bit
    return 2;
  } else if (frequency >= 5000000) {  // 5MHz - 10MHz: 3-bit
    return 3;
  } else if (frequency >= 2500000) {  // 2.5MHz - 5MHz: 4-bit
    return 4;
  } else if (frequency >= 1000000) {  // 1MHz - 2.5MHz: 5-bit
    return 5;
  } else if (frequency >= 500000) {   // 500kHz - 1MHz: 6-bit
    return 6;
  } else if (frequency >= 250000) {   // 250-500kHz: 7-bit
    return 7;
  } else if (frequency >= 125000) {   // 125-250kHz: 8-bit
    return 8;
  } else if (frequency >= 60000) {    // 60-125kHz: 9-bit
    return 9;
  } else {                            // <60kHz: 10-bit
    return 10;
  }
}

// Initialize LEDC PWM with current settings
void initPWM() {
  // Stop any existing PWM first
  ledc_stop(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, 0);
  
  // Configure timer
  ledc_timer_config_t timer_config = {
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .duty_resolution = (ledc_timer_bit_t)PWM_RESOLUTION,
      .timer_num = PWM_TIMER,
      .freq_hz = PWM_FREQ,
      .clk_cfg = LEDC_AUTO_CLK
  };
  
  // Direct call without error checking (for lab use)
  ledc_timer_config(&timer_config);
  
  // Configure channel
  ledc_channel_config_t channel_config = {
      .gpio_num = LED_OUTPUT_PIN,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .channel = PWM_CHANNEL,
      .timer_sel = PWM_TIMER,
      .duty = 0,
      .hpoint = 0
  };
  
  // Direct call without error checking (for lab use)
  ledc_channel_config(&channel_config);
}

// Set duty cycle using native LEDC
void setDuty(float inputDutyPercent) {
  current_duty_hardware_percent = inputDutyPercent;
  current_duty_input_percent = inputDutyPercent;

  // Special handling for 1-bit resolution (square wave)
  if (PWM_RESOLUTION == 1) {
    int duty_value = (current_duty_hardware_percent >= 50.0) ? 1 : 0;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty_value);
  } else {
    // Calculate duty value for multi-bit resolution
    int duty_value = (int)((current_duty_hardware_percent * max_duty_value) / 100.0);
    
    // For very high frequencies, avoid extreme duty cycles
    if (PWM_FREQ > 10000000) {
      if (duty_value == 0) duty_value = 1;
      if (duty_value == max_duty_value) duty_value = max_duty_value - 1;
    }
    
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty_value);
  }
  
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  Serial.println();
  Serial.println("ESP32 HIGH FREQUENCY PWM GENERATOR");
  Serial.println();
  
  // Initialize with default settings
  updatePWM();
  initPWM();
  
  // Set initial duty
  setDuty(current_duty_input_percent);
  
  printManual();
  printStatus();
}

void loop() {
  if (stringComplete) {
    processInput();
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

void processInput() {
  inputString.trim();
  if (inputString.length() == 0) return;

  // Frequency command
  if (inputString.startsWith("f") || inputString.startsWith("F")) {
    String freqStr = inputString.substring(1);
    freqStr.toUpperCase();

    long newFreq = 0;
    if (freqStr.endsWith("K")) {
      newFreq = freqStr.substring(0, freqStr.length() - 1).toInt() * 1000;
    } else if (freqStr.endsWith("M")) {
      newFreq = freqStr.substring(0, freqStr.length() - 1).toInt() * 1000000;
    } else {
      newFreq = freqStr.toInt();
    }

    if (newFreq >= MIN_FREQ && newFreq <= MAX_FREQ) {
      PWM_FREQ = newFreq;
      
      // Calculate new resolution
      updatePWM();
      
      // Re-initialize with new settings
      initPWM();
      
      // Restore duty cycle
      setDuty(current_duty_input_percent);
      
      printStatus();
    } else {
      Serial.print("ERROR: Frequency must be ");
      Serial.print(MIN_FREQ);
      Serial.print("Hz to ");
      Serial.print(MAX_FREQ);
      Serial.println("Hz");
    }
  }
  // Duty cycle command
  else if (inputString.endsWith("%")) {
    String dutyStr = inputString.substring(0, inputString.length() - 1);
    float dutyPercent = dutyStr.toFloat();

    if (dutyPercent >= 0 && dutyPercent <= 100) {
      setDuty(dutyPercent);
      printStatus();
    } else {
      Serial.println("ERROR: Duty cycle must be 0 - 100%");
    }
  }
  // Help command
  else if (inputString.equalsIgnoreCase("help") || inputString.equals("?")) {
    printManual();
  }
  // Status command
  else if (inputString.equalsIgnoreCase("status")) {
    printStatus();
  } else {
    Serial.println("Unknown command. Type 'help' for instructions.");
  }
}

// Update PWM resolution based on frequency
void updatePWM() {
  PWM_RESOLUTION = calculateResolution(PWM_FREQ);
  
  // Recalculate max_duty_value
  if (PWM_RESOLUTION == 1) {
    max_duty_value = 1;  // 1-bit has only 0 and 1
  } else {
    max_duty_value = (1 << PWM_RESOLUTION) - 1;
  }
}

void printStatus() {
  // Format frequency string
  String freqStr;
  if (PWM_FREQ >= 1000000) {
    freqStr = String(PWM_FREQ / 1000000.0, 3) + " MHz";
  } else if (PWM_FREQ >= 1000) {
    freqStr = String(PWM_FREQ / 1000.0, 1) + " kHz";
  } else {
    freqStr = String(PWM_FREQ) + " Hz";
  }

  Serial.println();

  Serial.print("Frequency: ");
  Serial.print(freqStr);
  // Add spaces for alignment
  for (int i = freqStr.length(); i < 20; i++) Serial.print(" ");
  
  Serial.print("Resolution: ");
  Serial.print(PWM_RESOLUTION);
  Serial.print("-bit");
  if (PWM_RESOLUTION == 1) {
    Serial.print(" (Square wave)          ");
  } else {
    Serial.print(" (");
    Serial.print(max_duty_value + 1);
    Serial.print(" steps)");
    // Add spaces for alignment
    int spaces = 25 - (String(PWM_RESOLUTION).length() + String(max_duty_value + 1).length() + 11);
    for (int i = 0; i < spaces; i++) Serial.print(" ");
  }
  
  Serial.print("Duty Cycle: ");
  Serial.print(current_duty_input_percent, 1);
  Serial.print("%");
  Serial.println();
}