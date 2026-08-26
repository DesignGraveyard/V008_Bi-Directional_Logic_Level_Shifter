//1kbyte data

#define MAX_STR_LEN 1100 
char receivedStr[MAX_STR_LEN];
int charIndex = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '{') { 
      charIndex = 0;
    } 
    else if (c == '}') { 
      receivedStr[charIndex] = '\0';
      // Blasting back 1000+ bytes takes about 1 second at 9600 baud
      Serial.print("{");
      Serial.print(receivedStr);
      Serial.print("}");
      charIndex = 0;
    } 
    else {
      // Safety check to prevent crashing the Nano
      if (charIndex < MAX_STR_LEN - 1) {
        receivedStr[charIndex++] = c;
      }
    }
  }
}