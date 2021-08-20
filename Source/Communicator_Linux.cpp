#include "Communicator.h"

struct Kontroller::Communicator::ImplData
{
};

Kontroller::Communicator::Communicator(Kontroller& owningKontroller)
   : kontroller(owningKontroller)
   , implData(std::make_unique<ImplData>())
{
}

Kontroller::Communicator::~Communicator()
{
   disconnect();
}

bool Kontroller::Communicator::isConnected() const
{
   return false;
}

bool Kontroller::Communicator::connect()
{
   return false;
}

void Kontroller::Communicator::disconnect()
{
}

bool Kontroller::Communicator::initializeMessage()
{
   return false;
}

bool Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes)
{
   return false;
}

bool Kontroller::Communicator::finalizeMessage()
{
   return false;
}
