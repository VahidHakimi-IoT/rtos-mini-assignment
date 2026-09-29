#define RGB_PIN 38
#define RGB_BRIGHTNESS 50

void taskA(void *parameter) {
  for (;;) {
    Serial.println("Task A alive");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
void taskB(void *parameter) {
  for (;;) {
    neopixelWrite(RGB_PIN, RGB_BRIGHTNESS, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    neopixelWrite(RGB_PIN, 0, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  xTaskCreatePinnedToCore(taskA, "Task A", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskB, "Task B", 2048, NULL, 1, NULL, 1);
}

void loop() {}