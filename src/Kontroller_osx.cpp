#include "Kontroller/Kontroller.h"

#include <CoreMIDI/MIDIServices.h>

#include <cassert>
#include <mutex>
#include <string>

#if !defined(KONTROLLER_ASSERT)
#  define KONTROLLER_ASSERT assert
#endif

#include "KontrollerIDs.inc"

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

   void enableLEDControl(MIDIPortRef outputPort, MIDIEndpointRef destination, bool enable) {
      struct StatePacketList {
         MIDIPacketList list;
         std::array<uint8_t, sizeof(MIDIPacket)> padding; // Need two packets worth of space (one in the list above)
      };
      const size_t size = sizeof(StatePacketList);

      StatePacketList packetList;
      MIDIPacket* packet = MIDIPacketListInit(&packetList.list);

      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kStartSysex.size(), kStartSysex.data());
      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kSecondSysex.size(), kSecondSysex.data());
      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kStartSysex.size(), kStartSysex.data());

      if (enable) {
         std::array<uint8_t, kMainSysex.size()> enableSysex(kMainSysex);
         enableSysex[kLEDModeOffset] = 0x01;
         packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, enableSysex.size(), enableSysex.data());
      } else {
         packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kMainSysex.size(), kMainSysex.data());
      }

      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kStartSysex.size(), kStartSysex.data());
      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kEndSysex.size(), kEndSysex.data());

      OSStatus sendResult = MIDISend(outputPort, destination, &packetList.list);
      KONTROLLER_ASSERT(sendResult == noErr);
   }

   void setLEDOn(MIDIPortRef outputPort, MIDIEndpointRef destination, ControlID id, bool enable) {
      MIDIPacketList packetList;
      MIDIPacket* packet = MIDIPacketListInit(&packetList);

      std::array<uint8_t, 3> data;
      data[0] = 0xB0;
      data[1] = id;
      data[2] = enable ? 0x7F : 0x00;

      packet = MIDIPacketListAdd(&packetList, sizeof(MIDIPacketList), packet, 0, data.size(), data.data());

      OSStatus sendResult = MIDISend(outputPort, destination, &packetList);
      KONTROLLER_ASSERT(sendResult == noErr);
   }
} // namespace

struct Kontroller::ImplData {
   std::mutex mutex;

   MIDIClientRef client { 0 };
   MIDIPortRef inputPort { 0 };
   MIDIPortRef outputPort { 0 };
   MIDIEndpointRef destination { 0 };
};

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

Kontroller::State Kontroller::getState() const {
   std::lock_guard<std::mutex> lock(data->mutex);
   return state;
}

void Kontroller::enableLEDControl(bool enable) {
   ::enableLEDControl(data->outputPort, data->destination, enable);
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on) {
   ::setLEDOn(data->outputPort, data->destination, idForLED(led), on);
}

void Kontroller::update(uint8_t id, uint8_t value) {
   std::lock_guard<std::mutex> lock(data->mutex);

   float *floatVal = getFloatVal(state, id);
   if (floatVal) {
      *floatVal = value / 127.0f;
   } else {
      bool *boolVal = getBoolVal(state, id);
      KONTROLLER_ASSERT(boolVal); // If it isn't a dial or slider, it should be a button

      *boolVal = value != 0;
   }
}
