// class VibroManager {
// private:
//     int pin;
//     unsigned long startTime;
//     int duration;
//     bool isActive;

// public:
//     VibroManager(int vibroPin) {
//         pin = vibroPin;
//         pinMode(pin, OUTPUT);
//         digitalWrite(pin, LOW);
//         isActive = false;
//     }

//     void start(int vibroDuration) {
//         digitalWrite(pin, HIGH);
//         startTime = millis();
//         duration = vibroDuration;
//         isActive = true;
//     }

//     void update() {
//         if (isActive && millis() - startTime >= duration) {
//             digitalWrite(pin, LOW);
//             isActive = false;
//         }
//     }

//     bool isVibrating() {
//         return isActive;
//     }
// };
