#include "Kontroller/Kontroller.h"
#include "Communicator.h"

struct Kontroller::Communicator::ImplData {
};

Kontroller::Communicator::Communicator(Kontroller* kontroller)
   : implData(new ImplData), kontroller(kontroller) {
}

Kontroller::Communicator::~Communicator() {
}

void Kontroller::Communicator::initializeMessage() {
}

void Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes) {
}

void Kontroller::Communicator::finalizeMessage() {
}
