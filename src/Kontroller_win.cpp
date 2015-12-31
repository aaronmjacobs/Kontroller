#include "Kontroller/Kontroller.h"

struct Kontroller::ImplData {
};

Kontroller::Kontroller() {
}

Kontroller::~Kontroller() {
}

Kontroller::State Kontroller::getState() const {
   return state;
}

void Kontroller::enableLEDControl(bool enable) {
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on) {
}

void Kontroller::update(uint8_t id, uint8_t value) {
}
