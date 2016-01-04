#include "Kontroller/Kontroller.h"

namespace CommunicatorCallback {

void receiveMessage(Kontroller *kontroller, uint8_t id, uint8_t value) {
   if (kontroller) {
      kontroller->update(id, value);
   }
}

} // namespace CommunicatorCallback

struct Kontroller::Communicator::ImplData {
};

Kontroller::Communicator::Communicator(Kontroller* kontroller)
   : implData(new ImplData) {
}

Kontroller::Communicator::~Communicator() {
}

void Kontroller::Communicator::initializeMessage() {
}

void Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes) {
}

void Kontroller::Communicator::finalizeMessage() {
}
