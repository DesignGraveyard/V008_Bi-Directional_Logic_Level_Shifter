//1kbyte data
#include <HardwareSerial.h>
HardwareSerial NanoSerial(2); 

char secretToken[1001]; // 1000 chars + null terminator
char responseBuffer[1100];
int successCount = 0;
int failCount = 0;

void setup() {
  Serial.begin(9600);
  NanoSerial.begin(9600, SERIAL_8N1, 16, 17);
  
  // Generate a 1000-character test pattern: "0123456789..."
  for (int i = 0; i < 1000; i++) {
    secretToken[i] = (i % 10) + '0'; 
  }
  secretToken[1000] = '\0';

  Serial.println("\n--- STARTING 1KB STRESS TEST ---");
}

void loop() {
  while(NanoSerial.available()) NanoSerial.read();

  Serial.println("Transmitting 1,000 bytes...");
  NanoSerial.print("{");
  NanoSerial.print(secretToken);
  NanoSerial.print("}");

  unsigned long startWait = millis();
  int idx = 0;
  bool complete = false;

  // At 9600 baud, 1000 bytes takes ~1.1 seconds to travel one way.
  // We wait 3 seconds total for the round trip.
  while (millis() - startWait < 3000) { 
    if (NanoSerial.available() > 0) {
      char c = NanoSerial.read();
      if (c == '{') { 
        idx = 0; 
      } else if (c == '}') {
        responseBuffer[idx] = '\0';
        complete = true;
        break;
      } else if (idx < 1099) {
        responseBuffer[idx++] = c;
      }
    }
  }

  if (complete && strcmp(responseBuffer, secretToken) == 0) {
    successCount++;
    Serial.print(">>> SUCCESS! Count: ");
  } else {
    failCount++;
    Serial.print(">>> FAIL! (Length Rcvd: ");
    Serial.print(idx);
    Serial.print(") Count: ");
  }
  
  Serial.print(successCount);
  Serial.print(" | Fails: ");
  Serial.println(failCount);

  // Give the level shifter a rest (2 seconds)
  delay(200); 
}