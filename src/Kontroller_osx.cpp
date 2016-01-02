#include "Kontroller/Kontroller.h"

#include <CoreMIDI/MIDIServices.h>

#include <cassert>
#include <string>

#if !defined(KONTROLLER_ASSERT)
#  define KONTROLLER_ASSERT assert
#endif

struct Kontroller::ImplData {
   MIDIClientRef client { 0 };
   MIDIPortRef inputPort { 0 };
   MIDIPortRef outputPort { 0 };
   MIDIEndpointRef destination { 0 };
};

namespace {

// Called on a separate thread
void midiInputCallback(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon) {
   Kontroller *kontroller = static_cast<Kontroller*>(readProcRefCon);
   if (!kontroller) {
      return;
   }

   const MIDIPacket *packet = &pktlist->packet[0];
   for (UInt32 i = 0; i < pktlist->numPackets; ++i) {
      if (packet->length == 3) {
         kontroller->update(packet->data[1], packet->data[2]);
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

struct SendInfo {
   MIDIPortRef outputPort { 0 };
   MIDIEndpointRef destination { 0 };

   union {
      MIDIPacketList list;
      std::array<uint8_t, 512> padding; // Ensure the list has enough space
   };
   MIDIPacket* lastPacket { nullptr };
};

void initializeInfo(const Kontroller::ImplData& implData, SendInfo* info) {
   info->outputPort = implData.outputPort;
   info->destination = implData.destination;

   info->lastPacket = MIDIPacketListInit(&info->list);
   KONTROLLER_ASSERT(info->lastPacket);
}

void finalizeInfo(SendInfo* info) {
   OSStatus sendResult = MIDISend(info->outputPort, info->destination, &info->list);
   KONTROLLER_ASSERT(sendResult == noErr);
}

template<size_t numBytes>
void send(SendInfo* info, std::array<uint8_t, numBytes> data) {
   info->lastPacket = MIDIPacketListAdd(&info->list, info->padding.size(), info->lastPacket, 0, data.size(), data.data());
}

} // namespace

Kontroller::Kontroller() {
   data = std::unique_ptr<ImplData>(new ImplData);

   Endpoints endpoints = findEndpoint();
   KONTROLLER_ASSERT(endpoints.source && endpoints.destination);
   data->destination = endpoints.destination;

   OSStatus clientResult = MIDIClientCreate(CFSTR("Kontroller client"), nullptr, nullptr, &data->client);
   KONTROLLER_ASSERT(clientResult == noErr);

   OSStatus inputPortResult = MIDIInputPortCreate(data->client, CFSTR("Kontroller input port"),
                                                  midiInputCallback, this, &data->inputPort);
   KONTROLLER_ASSERT(inputPortResult == noErr);

   OSStatus outputPortResult = MIDIOutputPortCreate(data->client, CFSTR("Kontroller output port"), &data->outputPort);
   KONTROLLER_ASSERT(outputPortResult == noErr);

   OSStatus connectResult = MIDIPortConnectSource(data->inputPort, endpoints.source, nullptr);
   KONTROLLER_ASSERT(connectResult == noErr);
}

Kontroller::~Kontroller() {
   if (data->client) {
      MIDIClientDispose(data->client); // Closes the ports as well
   }
}

#include "Kontroller_common.inc"
