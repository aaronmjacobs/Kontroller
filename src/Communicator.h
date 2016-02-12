#ifndef KONTROLLER_COMMUNICATOR_H
#define KONTROLLER_COMMUNICATOR_H

#include "Kontroller/Kontroller.h"

class Kontroller::Communicator {
private:
   struct ImplData;

   std::unique_ptr<ImplData> implData;
   Kontroller* kontroller {};
   bool notifiedOfLostConnection {};
   std::mutex lostConnectionMutex;

   bool appendToMessage(uint8_t* data, size_t numBytes);

   void checkForLostConnection() {
      std::lock_guard<std::mutex> lock(lostConnectionMutex);

      if (notifiedOfLostConnection) {
         notifiedOfLostConnection = false;
         disconnect();
      }
   }

public:
   Communicator(Kontroller* kontroller);

   ~Communicator();

   const ImplData& getImplData() const {
      return *implData;
   }

   bool isConnected() const;

   bool connect();

   void disconnect();

   void poll();

   bool initializeMessage();

   template<size_t numBytes>
   bool appendToMessage(std::array<uint8_t, numBytes> data) {
      return appendToMessage(data.data(), data.size());
   }

   bool finalizeMessage();

   void onMessageReceived(uint8_t id, uint8_t value) {
      kontroller->update(id, value);
   }

   void onConnectionLost() {
      std::lock_guard<std::mutex> lock(lostConnectionMutex);
      notifiedOfLostConnection = true;
   }
};

#endif
