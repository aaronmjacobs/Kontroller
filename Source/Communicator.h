#pragma once

#include "Kontroller/Device.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

namespace Kontroller
{
   class Device::Communicator
   {
   public:
      struct ImplData;

      Communicator(Device& owningDevice);
      ~Communicator();

      const ImplData& getImplData() const
      {
         return *implData;
      }

      bool isConnected() const;

      bool connect();
      void disconnect();
      void poll();

      bool initializeMessage();

      template<size_t NumBytes>
      bool appendToMessage(std::array<uint8_t, NumBytes> data)
      {
         return appendToMessage(data.data(), data.size());
      }

      bool appendToMessage(uint8_t* data, size_t numBytes);

      bool finalizeMessage();

      void onMessageReceived(uint8_t id, uint8_t value)
      {
         device.queueMessage(id, value);
      }

      void onConnectionLost()
      {
         notifiedOfLostConnection.store(true);
      }

   private:
      Device& device;
      std::unique_ptr<ImplData> implData;

      std::atomic_bool notifiedOfLostConnection = { false };

      void checkForLostConnection()
      {
         bool connectionLost = notifiedOfLostConnection.exchange(false);
         if (connectionLost)
         {
            disconnect();
         }
      }
   };
}
