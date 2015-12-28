#include "Kontroller/Kontroller.h"

Kontroller::Kontroller() {
}

Kontroller::~Kontroller() {
}

KontrolState Kontroller::getState() const {
   return state;
}

void Kontroller::update(unsigned char id, unsigned char value) {
}
