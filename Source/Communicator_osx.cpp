#include "Kontroller/Kontroller.h"
#include "Communicator.h"

#include <CoreMIDI/MIDIServices.h>

#include <string>

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

namespace {

// Called on the main thread
void midiNotifyCallback(const MIDINotification *message, void *refCon) {
   if (message->messageID == kMIDIMsgObjectRemoved) {
      const MIDIObjectAddRemoveNotification* notification =
         reinterpret_cast<const MIDIObjectAddRemoveNotification*>(message);
      Kontroller::Communicator* communicator = reinterpret_cast<Kontroller::Communicator*>(refCon);

      if (notification->child == communicator->getImplData().destination) {
         communicator->onConnectionLost();
      }
   }
}

// Called on a separate thread
void midiInputCallback(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon) {
   const MIDIPacket *packet = &pktlist->packet[0];
   for (UInt32 i = 0; i < pktlist->numPackets; ++i) {
      if (packet->length == 3) {
         reinterpret_cast<Kontroller::Communicator*>(readProcRefCon)->onMessageReceived(packet->data[1], packet->data[2]);
      }
      packet = MIDIPacketNext(packet);
   }
}

struct Endpoints {
   MIDIEndpointRef source;
   MIDIEndpointRef destination;
};

Endpoints findEndpoints(const char* deviceName) {
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
      if (std::string(cstr) != deviceName) {
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

      SInt32 offline = 0;
      MIDIObjectGetIntegerProperty(device, kMIDIPropertyOffline, &offline);
      if (offline) {
         continue;
      }

      endpoints.source = MIDIEntityGetSource(entity, 0);
      endpoints.destination = MIDIEntityGetDestination(entity, 0);
      break;
   }

   return endpoints;
}

} // namespace

Kontroller::Communicator::Communicator(Kontroller* kontroller)
   : implData(new ImplData), kontroller(kontroller) {
}

Kontroller::Communicator::~Communicator() {
   disconnect();
}

bool Kontroller::Communicator::isConnected() const {
   return implData->client && implData->inputPort && implData->outputPort && implData->destination;
}

bool Kontroller::Communicator::connect() {
   if (isConnected()) {
      return true;
   }

   bool success = false;
   do {
      Endpoints endpoints = findEndpoints(Kontroller::kDeviceName);
      if (!endpoints.source || !endpoints.destination) {
         break;
      }
      implData->destination = endpoints.destination;

      OSStatus clientResult = MIDIClientCreate(CFSTR("Kontroller client"), midiNotifyCallback, this, &implData->client);
      if (clientResult != noErr) {
         break;
      }

      OSStatus inputPortResult = MIDIInputPortCreate(implData->client, CFSTR("Kontroller input port"),
                                                     midiInputCallback, this, &implData->inputPort);
      if (inputPortResult != noErr) {
         break;
      }

      OSStatus outputPortResult = MIDIOutputPortCreate(implData->client, CFSTR("Kontroller output port"),
                                                       &implData->outputPort);
      if (outputPortResult != noErr) {
         break;
      }

      OSStatus connectResult = MIDIPortConnectSource(implData->inputPort, endpoints.source, nullptr);
      if (connectResult != noErr) {
         break;
      }

      success = true;
   } while(false);

   if (!success) {
      disconnect();
   }

   return success;
}

void Kontroller::Communicator::disconnect() {
   if (implData->client) {
      MIDIClientDispose(implData->client); // Closes the ports as well
   }

   *implData = {};
}

void Kontroller::Communicator::poll() {
   CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);

   // We will only be notified of a lost connection when the main loop is run (that is the only time the notify callback
   // is executed), so we only need to check for lost connections here
   checkForLostConnection();
}

bool Kontroller::Communicator::initializeMessage() {
   implData->lastPacket = MIDIPacketListInit(&implData->list);
   return implData->lastPacket != nullptr;
}

bool Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes) {
   implData->lastPacket = MIDIPacketListAdd(&implData->list, implData->padding.size(), implData->lastPacket,
                                            0, numBytes, data);
   return implData->lastPacket != nullptr;
}

bool Kontroller::Communicator::finalizeMessage() {
   OSStatus sendResult = MIDISend(implData->outputPort, implData->destination, &implData->list);
   return sendResult == noErr;
}
