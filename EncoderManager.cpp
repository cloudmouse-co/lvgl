#include "EncoderManager.h"

// Inizializziamo l'encoder sui pin corretti
EncoderManager::EncoderManager()
  : encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN), lastPosition(0) {
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
}

void EncoderManager::startEncoderHandlerTask() {
  // Crea il task solo se non è già attivo
  if (encoderTaskHandle == NULL) {
    xTaskCreatePinnedToCore(
      this->encoderHandlerTask,   // Task per gestire l'animazione
      "Encoder Task",      // Nome del task
      4096,                  // Stack size
      this,                  // Parametro (istanza LEDManager)
      2,                     // Priorità
      &encoderTaskHandle,  // Handle del task
      1                      // Core su cui far girare il task (1)
    );
  }
}

void EncoderManager::encoderHandlerTask(void* parameter) {
  EncoderManager* self = static_cast<EncoderManager*>(parameter);
  while (true) {
    self->update();
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Breve delay per evitare consumo eccessivo di CPU
  }
}

void EncoderManager::update() {
  lastTimePressed = timePressed();
  clicked = 0 < lastTimePressed && lastTimePressed < clickTimeout;
  longPressed = lastTimePressed >= longPressDuration;
}

int EncoderManager::getMovement() {
  int currentPosition = encoder.position() / 4;
  int movement = currentPosition - lastPosition;
  lastPosition = currentPosition;
  return movement;
}

long EncoderManager::timePressed() {
  // Leggi lo stato attuale del pulsante
  bool currentButtonState = digitalRead(ENCODER_SW_PIN);

  // Ottieni il tempo attuale
  unsigned long currentTime = millis();

  // Se il pulsante è stato rilasciato (transizione da LOW a HIGH)
  if (currentButtonState != lastButtonState && currentButtonState == HIGH) {
    unsigned long pressDuration = currentTime - lastPressTime;
    // Calcola la durata della pressione

    // Serial.print("elapsed time pressed: ");
    // Serial.println(pressDuration);

    lastButtonState = currentButtonState;
    return pressDuration;
  }

  // Se il pulsante è appena stato premuto, aggiorna il tempo dell'ultimo click
  if (currentButtonState != lastButtonState && currentButtonState == LOW) {
    lastPressTime = currentTime;
  }

  // Aggiorna lo stato del pulsante
  lastButtonState = currentButtonState;

  return 0;  // Altrimenti, ritorna false
}