#include "Kontroller/Kontroller.h"

#include <CoreMIDI/MIDIServices.h>

#include <string>

namespace {

// Called on a separate thread
void midiInputCallback(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon) {
   const MIDIPacket *packet = &pktlist->packet[0];
   for (UInt32 i = 0; i < pktlist->numPackets; ++i) {
      if (packet->length == 3) {
         CommunicatorCallback::receiveMessage(static_cast<Kontroller*>(readProcRefCon),
                                              packet->data[1], packet->data[2]);
      }
      packet = MIDIPacketNext(packet);
   }
}

struct Endpoints {
   MIDIEndpointRef source;
   MIDIEndpointRef destination;
};

Endpoints findEndpoint() {
   Endpoints endpoints { 0 };
   ItemCount deviceCount = MIDIGetNumberOfDevices();

   for (ItemCount i = 0; i < deviceCount; ++i) {
      MIDIDeviceRef device = MIDIGetDevice(i);

      CFStringRef name = nullptr;
      OSStatus nameResult = MIDIObjectGetStringProperty(device, kMIDIPropertyName, &name);
      if (nameResult != noErr) {
         continue;
      }
      const char *cstr = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
      CFRelease(name);
      if (!cstr) {
         continue;
      }
      if (std::string(cstr) != "nanoKONTROL2") {
         continue;
      }

      ItemCount entityCount = MIDIDeviceGetNumberOfEntities(device);
      if (entityCount != 1) {
         continue;
      }

      MIDIEntityRef entity = MIDIDeviceGetEntity(device, 0);
      ItemCount sourceCount = MIDIEntityGetNumberOfSources(entity);
      if (sourceCount != 1) {
         continue;
      }
      ItemCount destinationCount = MIDIEntityGetNumberOfDestinations(entity);
      if (destinationCount != 1) {
         continue;
      }

      endpoints.source = MIDIEntityGetSource(entity, 0);
      endpoints.destination = MIDIEntityGetDestination(entity, 0);

      SInt32 offline = 0;
      MIDIObjectGetIntegerProperty(device, kMIDIPropertyOffline, &offline);
      if (!offline) {
         break; // Take the first online device (but fall back to an offline one)
      }
   }

   return endpoints;
}

} // namespace

namespace CommunicatorCallback {

void receiveMessage(Kontroller *kontroller, uint8_t id, uint8_t value) {
   if (kontroller) {
      kontroller->update(id, value);
   }
}

} // namespace CommunicatorCallback

struct Kontroller::Communicator::ImplData {
   MIDIClientRef client { 0 };
   MIDIPortRef inputPort { 0 };
   MIDIPortRef outputPort { 0 };
   MIDIEndpointRef destination { 0 };

   union {
      MIDIPacketList list;
      std::array<uint8_t, 512> padding; // Ensure the list has enough space
   };
   MIDIPacket* lastPacket { nullptr };
};

Kontroller::Communicator::Communicator(Kontroller* kontroller)
   : implData(new ImplData) {
   Endpoints endpoints = findEndpoint();
   KONTROLLER_ASSERT(endpoints.source && endpoints.destination);
   implData->destination = endpoints.destination;

   OSStatus clientResult = MIDIClientCreate(CFSTR("Kontroller client"), nullptr, nullptr, &implData->client);
   KONTROLLER_ASSERT(clientResult == noErr);

   OSStatus inputPortResult = MIDIInputPortCreate(implData->client, CFSTR("Kontroller input port"),
                                                  midiInputCallback, kontroller, &implData->inputPort);
   KONTROLLER_ASSERT(inputPortResult == noErr);

   OSStatus outputPortResult = MIDIOutputPortCreate(implData->client, CFSTR("Kontroller output port"),
                                                    &implData->outputPort);
   KONTROLLER_ASSERT(outputPortResult == noErr);

   OSStatus connectResult = MIDIPortConnectSource(implData->inputPort, endpoints.source, nullptr);
   KONTROLLER_ASSERT(connectResult == noErr);
}

Kontroller::Communicator::~Communicator() {
   if (implData->client) {
      MIDIClientDispose(implData->client); // Closes the ports as well
   }
}

void Kontroller::Communicator::initializeMessage() {
   implData->lastPacket = MIDIPacketListInit(&implData->list);
   KONTROLLER_ASSERT(implData->lastPacket);
}

void Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes) {
   implData->lastPacket = MIDIPacketListAdd(&implData->list, implData->padding.size(), implData->lastPacket,
                                            0, numBytes, data);
}

void Kontroller::Communicator::finalizeMessage() {
   OSStatus sendResult = MIDISend(implData->outputPort, implData->destination, &implData->list);
   KONTROLLER_ASSERT(sendResult == noErr);
}
