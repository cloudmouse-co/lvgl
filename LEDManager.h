//#include <sys/_stdint.h>

#include <stdint.h>

#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 12
#define DATA_PIN 15

extern Adafruit_NeoPixel ledStrip;

class LEDManager {
private:
  Adafruit_NeoPixel& strip;
  TaskHandle_t animationTaskHandle = NULL;

  bool pulsating = true;
  int cursorLED = 0;
  unsigned long previousMillis = 0;
  const unsigned long interval = 70;
  bool clockwise = true;
  unsigned long fadeStartMillis = 0;
  int targetBrightness = 0;
  int startBrightness = 0;
  int fadeDuration = 3000;
  bool fading = false;
  int currentBrightness = 250;
  bool inited = false;
  bool initAnimationCompleted = false;

  unsigned long lastEncMovementTime = 0;
  int IDLE_DALY_SECONDS = 5;

  bool flash = false;
  unsigned long lastFlashStarted = 0;
  unsigned long flashDuration = 0;

  bool loading = false;

  // Taskomat
  // uint8_t baseRed = 0;
  // uint8_t baseGreen = 254;
  // uint8_t baseBlue = 222;

  uint8_t baseRed = 0;
  uint8_t baseGreen = 181;
  uint8_t baseBlue = 214;


  // NIDO
  // uint8_t baseRed = 30;
  // uint8_t baseGreen = 254;
  // uint8_t baseBlue = 30;

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  static void animationTask(void* parameter) {
    LEDManager* self = static_cast<LEDManager*>(parameter);
    while (true) {
      self->update();
      vTaskDelay(2 / portTICK_PERIOD_MS);  // Breve delay per evitare consumo eccessivo di CPU
    }
  }

  void update() {
    unsigned long currentMillis = millis();

    if (fading) {
      // Se c'è un fade attivo, aggiorna il fade
      updateFade();
      return;
    }

    // Gestione animazione rotazione
    if (currentMillis - previousMillis >= interval && !inited) {
      previousMillis = currentMillis;

      reset();  // Resetta prima di cambiare colore al singolo LED
      strip.setPixelColor(cursorLED == strip.numPixels() ? 0 : cursorLED, strip.Color(red, green, blue));
      strip.show();

      if (clockwise) {
        cursorLED++;
      } else {
        cursorLED--;
      }

      // Cambia direzione quando raggiungi i limiti
      if (cursorLED == strip.numPixels()) {
        clockwise = false;
      }

      if (cursorLED == 0 && !clockwise) {
        // Fine dell'animazione, avvia il fade
        delay(interval);
        strip.setBrightness(0);
        strip.show();
        delay(500);
        currentBrightness = 0;
        fadeToBrightness(255, 150);
        inited = true;
      }

      if (cursorLED == 0) {
        clockwise = true;
      }
    }

    if (!initAnimationCompleted) {

      // init flash and fade out
      if (inited && getBrightness() != 0) {
        fadeToBrightness(0, 3000);
      }

      // set init animation as completed after 4 seconds
      if (millis() > 4000) {
        initAnimationCompleted = true;
      }

      return;
    }

    // reset flash color
    if (flash && millis() - lastFlashStarted >= flashDuration) {
      flash = false;
      flashDuration = 0;
      resetColor();
      strip.show();
    }

    // loading animation handler
    if (loading) {
      if (getBrightness() != 250) {
        fadeToBrightness(250, 50);
      }

      if (getBrightness() != 1) {
        fadeToBrightness(1, 50);
      }
    }

    // pulsating animation handler
    if (pulsating) {
      if (getBrightness() != 200) {
        fadeToBrightness(200, 1500);
      }

      if (getBrightness() != 25) {
        fadeToBrightness(25, 1500);
      }
    }

    // back to idle pulsating mode
    if (millis() - lastEncMovementTime >= IDLE_DALY_SECONDS * 1000 && !isPulsating()) {
      if (getBrightness() != 25) {
        fadeToBrightness(25, 1000);
      } else {
        setPulsating(true);
      }
    }
  }

public:
  LEDManager(Adafruit_NeoPixel& strip)
    : strip(strip) {}

  void setLoadingState(bool on) {
    if (!initAnimationCompleted) {
      return;
    }

    if (loading != on) {
      loading = on;

      if (loading) {
        lastEncMovementTime = millis();
        activate();
        setColor(244, 70, 17);
      } else {
        setColor(baseRed, baseGreen, baseBlue);
        strip.show();
        fadeToBrightness(200, 150);
        // setPulsating(true);
      }
    }
  }

  bool isLoading() {
    return loading;
  }

  void setPulsating(bool on) {
    pulsating = on;
  }

  bool isPulsating() {
    return pulsating;
  }

  int getBrightness() {
    return currentBrightness;
  }

  void init() {
    strip.begin();
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.setBrightness(currentBrightness);
    strip.show();

    red = baseRed;
    green = baseGreen;
    blue = baseBlue;
  }

  void resetColor() {
    setColor(baseRed, baseGreen, baseBlue);
  }

  void reset() {
    // Resetta tutti i LED a un colore di default
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }

  // Funzione per fare il fade della luminosità
  void fadeToBrightness(int brightness, int milliseconds) {
    startBrightness = currentBrightness;  // Usa la luminosità corrente
    targetBrightness = brightness;
    if (!fading) {
      fadeStartMillis = millis();
    }
    fadeDuration = milliseconds;
    fading = true;
  }

  // Funzione per impostare tutti i LED allo stesso colore
  void setColor(uint8_t r, uint8_t g, uint8_t b) {
    red = r;
    green = g;
    blue = b;

    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    // strip.show();
  }

  // Funzione per gestire il fade (interna)
  void updateFade() {
    if (fading) {
      unsigned long elapsedTime = millis() - fadeStartMillis;

      if (elapsedTime <= fadeDuration) {
        int brightness = map(elapsedTime, 0, fadeDuration, startBrightness, targetBrightness);
        strip.setBrightness(brightness);
        for (int i = 0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(red, green, blue));
        }
        strip.show();
      } else {
        // Finito il fade
        fading = false;
        currentBrightness = targetBrightness;
        strip.setBrightness(currentBrightness);
        for (int i = 0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(red, green, blue));
        }
        strip.show();
      }
    }
  }

  void startAnimationTask() {
    // Crea il task solo se non è già attivo
    if (animationTaskHandle == NULL) {
      xTaskCreatePinnedToCore(
        this->animationTask,   // Task per gestire l'animazione
        "Animation Task",      // Nome del task
        4096,                  // Stack size
        this,                  // Parametro (istanza LEDManager)
        6,                     // Priorità
        &animationTaskHandle,  // Handle del task
        0                      // Core su cui far girare il task (1)
      );
    }
  }

  void flashColor(uint8_t r, uint8_t g, uint8_t b, int brightness, int millisDuration) {
    setPulsating(false);
    updateLastEncoderMovementTime();
    fading = false;
    currentBrightness = brightness;

    flash = true;
    flashDuration = millisDuration;
    lastFlashStarted = millis();

    vTaskDelay(10);

    strip.setBrightness(brightness);
    setColor(r, g, b);
    strip.show();
  }

  void updateLastEncoderMovementTime() {
    if (!loading) {
      lastEncMovementTime = millis();
    }
  }

  void activate() {
    setPulsating(false);
    updateLastEncoderMovementTime();

    fading = false;
    currentBrightness = 255;

    vTaskDelay(10);

    strip.setBrightness(255);
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    strip.show();
  }
};

#endif