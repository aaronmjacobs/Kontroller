#ifndef KONTROLLER_COMMUNICATOR_H
#define KONTROLLER_COMMUNICATOR_H

#include "Kontroller/Kontroller.h"

class Kontroller::Communicator {
private:
   struct ImplData;

   std::unique_ptr<ImplData> implData;
   Kontroller* kontroller;

   bool appendToMessage(uint8_t* data, size_t numBytes);

public:
   Communicator(Kontroller* kontroller);

   ~Communicator();

   bool isConnected() const;

   bool connect();

   void disconnect();

   bool initializeMessage();

   template<size_t numBytes>
   bool appendToMessage(std::array<uint8_t, numBytes> data) {
      return appendToMessage(data.data(), data.size());
   }

   bool finalizeMessage();

   void onMessageReceived(uint8_t id, uint8_t value) {
      kontroller->update(id, value);
   }
};

#endif
