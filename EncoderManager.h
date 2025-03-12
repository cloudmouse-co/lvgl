#ifndef ENCODERMANAGER_H
#define ENCODERMANAGER_H

#include <RotaryEncoderPCNT.h>
#include <Arduino.h>

#define ENCODER_CLK_PIN 16
#define ENCODER_DT_PIN 18
#define ENCODER_SW_PIN 17

class EncoderManager {
  public:
    EncoderManager();

    void update();

    void startEncoderHandlerTask();

    int getMovement();  // Restituisce il movimento dell'encoder (+1 o -1)
    bool isClicked() { return clicked; }
    bool isLongPressed() { return longPressed; }

    long getLastPressDuration() { return lastTimePressed; }
    int isButtonDown() { return digitalRead(ENCODER_SW_PIN) == LOW; }

    bool setLongPressDuration(int seconds) { longPressDuration = seconds * 1000; }
    bool setClickTimeout(int millis) { clickTimeout = millis; }


  private:
    RotaryEncoderPCNT encoder;
    TaskHandle_t encoderTaskHandle = NULL;

    static void encoderHandlerTask(void* parameter);
    long timePressed();

    // rotation management
    int lastPosition;

    // button management
    unsigned long lastTimePressed = 0;
    bool clicked = false;
    bool longPressed = false;
    bool lastButtonState = HIGH;        // Stato precedente del pulsante
    unsigned long lastPressTime = 0;    // Tempo dell'ultima pressione

    // config
    int longPressDuration = 1000;
    unsigned long clickTimeout = 500;

};

#endif
