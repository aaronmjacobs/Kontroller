#include "Communicator.h"

namespace Kontroller
{
   struct Device::Communicator::ImplData
   {
   };

   Device::Communicator::Communicator(Device& owningDevice)
      : device(owningDevice)
      , implData(std::make_unique<ImplData>())
   {
   }

   Device::Communicator::~Communicator()
   {
      disconnect();
   }

   bool Device::Communicator::isConnected() const
   {
      return false;
   }

   bool Device::Communicator::connect()
   {
      return false;
   }

   void Device::Communicator::disconnect()
   {
   }

   bool Device::Communicator::initializeMessage()
   {
      return false;
   }

   bool Device::Communicator::appendToMessage(uint8_t* data, size_t numBytes)
   {
      return false;
   }

   bool Device::Communicator::finalizeMessage()
   {
      return false;
   }
}
