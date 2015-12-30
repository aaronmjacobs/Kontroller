#include "Kontroller/Kontroller.h"

#include <CoreMIDI/MIDIServices.h>

#include <cassert>
#include <mutex>
#include <string>

#if !defined(KONTROLLER_ASSERT)
#  define KONTROLLER_ASSERT assert
#endif

namespace {
   enum ControlID : uint8_t {
      kTrackLeft = 0x3A,
      kTrackRight = 0x3B,
      kCycle = 0x2E,
      kMarkerSet = 0x3C,
      kMarkerLeft = 0x3D,
      kMarkerRight = 0x3E,
      kRewind = 0x2B,
      kFastForward = 0x2C,
      kStop = 0x2A,
      kPlay = 0x29,
      kRecord = 0x2D,
      kCol1Dial = 0x10,
      kCol1Slider = 0x00,
      kCol1S = 0x20,
      kCol1M = 0x30,
      kCol1R = 0x40,
      kCol2Dial = kCol1Dial + 1,
      kCol2Slider = kCol1Slider + 1,
      kCol2S = kCol1S + 1,
      kCol2M = kCol1M + 1,
      kCol2R = kCol1R + 1,
      kCol3Dial = kCol2Dial + 1,
      kCol3Slider = kCol2Slider + 1,
      kCol3S = kCol2S + 1,
      kCol3M = kCol2M + 1,
      kCol3R = kCol2R + 1,
      kCol4Dial = kCol3Dial + 1,
      kCol4Slider = kCol3Slider + 1,
      kCol4S = kCol3S + 1,
      kCol4M = kCol3M + 1,
      kCol4R = kCol3R + 1,
      kCol5Dial = kCol4Dial + 1,
      kCol5Slider = kCol4Slider + 1,
      kCol5S = kCol4S + 1,
      kCol5M = kCol4M + 1,
      kCol5R = kCol4R + 1,
      kCol6Dial = kCol5Dial + 1,
      kCol6Slider = kCol5Slider + 1,
      kCol6S = kCol5S + 1,
      kCol6M = kCol5M + 1,
      kCol6R = kCol5R + 1,
      kCol7Dial = kCol6Dial + 1,
      kCol7Slider = kCol6Slider + 1,
      kCol7S = kCol6S + 1,
      kCol7M = kCol6M + 1,
      kCol7R = kCol6R + 1,
      kCol8Dial = kCol7Dial + 1,
      kCol8Slider = kCol7Slider + 1,
      kCol8S = kCol7S + 1,
      kCol8M = kCol7M + 1,
      kCol8R = kCol7R + 1,
   };

   const std::array<uint8_t, 6> kStartSysex {{
      0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7
   }};

   const std::array<uint8_t, 402> kMainSysex {{
      0xF0, 0x42, 0x40, 0x00, 0x01, 0x13, 0x00, 0x7F, 0x7F, 0x02, 0x03, 0x05, 0x40, 0x00, 0x00, 0x00,
      0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x7F, 0x00,
      0x01, 0x00, 0x20, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x30, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00,
      0x40, 0x00, 0x7F, 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x11,
      0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x21, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x31, 0x00, 0x00, 0x7F,
      0x00, 0x01, 0x00, 0x41, 0x00, 0x00, 0x7F, 0x00, 0x10, 0x01, 0x00, 0x02, 0x00, 0x00, 0x7F, 0x00,
      0x01, 0x00, 0x12, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x22, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00,
      0x32, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x42, 0x00, 0x7F, 0x00, 0x10, 0x01, 0x00, 0x00, 0x03,
      0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x13, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x23, 0x00, 0x00, 0x7F,
      0x00, 0x01, 0x00, 0x33, 0x00, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x43, 0x00, 0x7F, 0x00, 0x00, 0x10,
      0x01, 0x00, 0x04, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x14, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00,
      0x24, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x34, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x44, 0x00,
      0x7F, 0x00, 0x10, 0x01, 0x00, 0x00, 0x05, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x15, 0x00, 0x00, 0x7F,
      0x00, 0x01, 0x00, 0x25, 0x00, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x35, 0x00, 0x7F, 0x00, 0x00, 0x01,
      0x00, 0x45, 0x00, 0x7F, 0x00, 0x00, 0x10, 0x01, 0x00, 0x06, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00,
      0x16, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x26, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x36, 0x00,
      0x7F, 0x00, 0x01, 0x00, 0x46, 0x00, 0x00, 0x7F, 0x00, 0x10, 0x01, 0x00, 0x07, 0x00, 0x00, 0x7F,
      0x00, 0x01, 0x00, 0x17, 0x00, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x27, 0x00, 0x7F, 0x00, 0x00, 0x01,
      0x00, 0x37, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x47, 0x00, 0x7F, 0x00, 0x10, 0x00, 0x01, 0x00,
      0x3A, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x3B, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x2E, 0x00,
      0x7F, 0x00, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x3D, 0x00, 0x00, 0x7F, 0x00,
      0x01, 0x00, 0x3E, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x2B, 0x00, 0x7F, 0x00, 0x00, 0x01, 0x00,
      0x2C, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x2A, 0x00, 0x7F, 0x00, 0x01, 0x00, 0x00, 0x29, 0x00,
      0x7F, 0x00, 0x01, 0x00, 0x2D, 0x00, 0x00, 0x7F, 0x00, 0x7F, 0x7F, 0x7F, 0x7F, 0x00, 0x7F, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xF7
   }};

   const std::array<uint8_t, 11> kSecondSysex {{
      0xF0, 0x42, 0x40, 0x00, 0x01, 0x13, 0x00, 0x1F, 0x12, 0x00, 0xF7
   }};

   const std::array<uint8_t, 11> kEndSysex {{
      0xF0, 0x42, 0x40, 0x00, 0x01, 0x13, 0x00, 0x1F, 0x11, 0x00, 0xF7
   }};

   const size_t kLedModeOffset = 16;

   ControlID idForLed(KontrolLed led) {
      switch (led) {
         case KontrolLed::kCycle: return kCycle;
         case KontrolLed::kRewind: return kRewind;
         case KontrolLed::kFastForward: return kFastForward;
         case KontrolLed::kStop: return kStop;
         case KontrolLed::kPlay: return kPlay;
         case KontrolLed::kRecord: return kRecord;
         case KontrolLed::kCol1S: return kCol1S;
         case KontrolLed::kCol1M: return kCol1M;
         case KontrolLed::kCol1R: return kCol1R;
         case KontrolLed::kCol2S: return kCol2S;
         case KontrolLed::kCol2M: return kCol2M;
         case KontrolLed::kCol2R: return kCol2R;
         case KontrolLed::kCol3S: return kCol3S;
         case KontrolLed::kCol3M: return kCol3M;
         case KontrolLed::kCol3R: return kCol3R;
         case KontrolLed::kCol4S: return kCol4S;
         case KontrolLed::kCol4M: return kCol4M;
         case KontrolLed::kCol4R: return kCol4R;
         case KontrolLed::kCol5S: return kCol5S;
         case KontrolLed::kCol5M: return kCol5M;
         case KontrolLed::kCol5R: return kCol5R;
         case KontrolLed::kCol6S: return kCol6S;
         case KontrolLed::kCol6M: return kCol6M;
         case KontrolLed::kCol6R: return kCol6R;
         case KontrolLed::kCol7S: return kCol7S;
         case KontrolLed::kCol7M: return kCol7M;
         case KontrolLed::kCol7R: return kCol7R;
         case KontrolLed::kCol8S: return kCol8S;
         case KontrolLed::kCol8M: return kCol8M;
         case KontrolLed::kCol8R: return kCol8R;
         default: KONTROLLER_ASSERT(false); return kTrackLeft;
      }
   }

   float* getFloatVal(KontrolState &state, uint8_t id) {
      switch (id) {
         case kCol1Dial: return &state.columns[0].dial;
         case kCol1Slider: return &state.columns[0].slider;
         case kCol2Dial: return &state.columns[1].dial;
         case kCol2Slider: return &state.columns[1].slider;
         case kCol3Dial: return &state.columns[2].dial;
         case kCol3Slider: return &state.columns[2].slider;
         case kCol4Dial: return &state.columns[3].dial;
         case kCol4Slider: return &state.columns[3].slider;
         case kCol5Dial: return &state.columns[4].dial;
         case kCol5Slider: return &state.columns[4].slider;
         case kCol6Dial: return &state.columns[5].dial;
         case kCol6Slider: return &state.columns[5].slider;
         case kCol7Dial: return &state.columns[6].dial;
         case kCol7Slider: return &state.columns[6].slider;
         case kCol8Dial: return &state.columns[7].dial;
         case kCol8Slider: return &state.columns[7].slider;
         default: return nullptr;
      }
   }

   bool* getBoolVal(KontrolState &state, uint8_t id) {
      switch (id) {
         case kTrackLeft: return &state.trackLeft;
         case kTrackRight: return &state.trackRight;
         case kCycle: return &state.cycle;
         case kMarkerSet: return &state.markerSet;
         case kMarkerLeft: return &state.markerLeft;
         case kMarkerRight: return &state.markerRight;
         case kRewind: return &state.rewind;
         case kFastForward: return &state.fastForward;
         case kStop: return &state.stop;
         case kPlay: return &state.play;
         case kRecord: return &state.record;
         case kCol1S: return &state.columns[0].s;
         case kCol1M: return &state.columns[0].m;
         case kCol1R: return &state.columns[0].r;
         case kCol2S: return &state.columns[1].s;
         case kCol2M: return &state.columns[1].m;
         case kCol2R: return &state.columns[1].r;
         case kCol3S: return &state.columns[2].s;
         case kCol3M: return &state.columns[2].m;
         case kCol3R: return &state.columns[2].r;
         case kCol4S: return &state.columns[3].s;
         case kCol4M: return &state.columns[3].m;
         case kCol4R: return &state.columns[3].r;
         case kCol5S: return &state.columns[4].s;
         case kCol5M: return &state.columns[4].m;
         case kCol5R: return &state.columns[4].r;
         case kCol6S: return &state.columns[5].s;
         case kCol6M: return &state.columns[5].m;
         case kCol6R: return &state.columns[5].r;
         case kCol7S: return &state.columns[6].s;
         case kCol7M: return &state.columns[6].m;
         case kCol7R: return &state.columns[6].r;
         case kCol8S: return &state.columns[7].s;
         case kCol8M: return &state.columns[7].m;
         case kCol8R: return &state.columns[7].r;
         default: return nullptr;
      }
   }

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

   void enableLedControl(MIDIPortRef outputPort, MIDIEndpointRef destination, bool enable) {
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
         enableSysex[kLedModeOffset] = 0x01;
         packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, enableSysex.size(), enableSysex.data());
      } else {
         packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kMainSysex.size(), kMainSysex.data());
      }

      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kStartSysex.size(), kStartSysex.data());
      packet = MIDIPacketListAdd(&packetList.list, size, packet, 0, kEndSysex.size(), kEndSysex.data());

      OSStatus sendResult = MIDISend(outputPort, destination, &packetList.list);
      KONTROLLER_ASSERT(sendResult == noErr);
   }

   void setLedOn(MIDIPortRef outputPort, MIDIEndpointRef destination, ControlID id, bool enable) {
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

struct KontrolImplData {
   std::mutex mutex;

   MIDIClientRef client { 0 };
   MIDIPortRef inputPort { 0 };
   MIDIPortRef outputPort { 0 };
   MIDIEndpointRef destination { 0 };
};

Kontroller::Kontroller() {
   data = std::unique_ptr<KontrolImplData>(new KontrolImplData);

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

KontrolState Kontroller::getState() const {
   std::lock_guard<std::mutex> lock(data->mutex);
   return state;
}

void Kontroller::enableLedControl(bool enable) {
   ::enableLedControl(data->outputPort, data->destination, enable);
}

void Kontroller::setLedOn(KontrolLed led, bool on) {
   ::setLedOn(data->outputPort, data->destination, idForLed(led), on);
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