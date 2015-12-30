#include "Kontroller/Kontroller.h"

Kontroller::Kontroller() {
}

Kontroller::~Kontroller() {
}

KontrolState Kontroller::getState() const {
   return state;
}

void Kontroller::enableLedControl(bool enable) {
}

void Kontroller::setLedOn(KontrolLed led, bool on) {
}

void Kontroller::update(uint8_t id, uint8_t value) {
}
