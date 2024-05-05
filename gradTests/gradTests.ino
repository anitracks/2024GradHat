/* gradTests
 * 
 * Test more of the grad board v0.1 2023
 *
 * Seth McNeill
 * 2023 January 20
 */



void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");
}

void loop() {
  int vbat = analogRead(A0);
  Serial.print(vbat);
  Serial.print(",");
  Serial.println((float)vbat*49.0/10.0);
  delay(1000);
}
