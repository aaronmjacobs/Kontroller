#include "Kontroller/Kontroller.h"

#include <string>

namespace {
   enum ControlID {
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

   float* getFloatVal(KontrolState &state, unsigned char id) {
      switch (id) {
         case kCol1Dial:
            return &state.columns[0].dial;
         case kCol1Slider:
            return &state.columns[0].slider;
         case kCol2Dial:
            return &state.columns[1].dial;
         case kCol2Slider:
            return &state.columns[1].slider;
         case kCol3Dial:
            return &state.columns[2].dial;
         case kCol3Slider:
            return &state.columns[2].slider;
         case kCol4Dial:
            return &state.columns[3].dial;
         case kCol4Slider:
            return &state.columns[3].slider;
         case kCol5Dial:
            return &state.columns[4].dial;
         case kCol5Slider:
            return &state.columns[4].slider;
         case kCol6Dial:
            return &state.columns[5].dial;
         case kCol6Slider:
            return &state.columns[5].slider;
         case kCol7Dial:
            return &state.columns[6].dial;
         case kCol7Slider:
            return &state.columns[6].slider;
         case kCol8Dial:
            return &state.columns[7].dial;
         case kCol8Slider:
            return &state.columns[7].slider;
         default:
            return nullptr;
      }
   }

   bool* getBoolVal(KontrolState &state, unsigned char id) {
      switch (id) {
         case kTrackLeft:
            return &state.trackLeft;
         case kTrackRight:
            return &state.trackRight;
         case kCycle:
            return &state.cycle;
         case kMarkerSet:
            return &state.markerSet;
         case kMarkerLeft:
            return &state.markerLeft;
         case kMarkerRight:
            return &state.markerRight;
         case kRewind:
            return &state.rewind;
         case kFastForward:
            return &state.fastForward;
         case kStop:
            return &state.stop;
         case kPlay:
            return &state.play;
         case kRecord:
            return &state.record;
         case kCol1S:
            return &state.columns[0].s;
         case kCol1M:
            return &state.columns[0].m;
         case kCol1R:
            return &state.columns[0].r;
         case kCol2S:
            return &state.columns[1].s;
         case kCol2M:
            return &state.columns[1].m;
         case kCol2R:
            return &state.columns[1].r;
         case kCol3S:
            return &state.columns[2].s;
         case kCol3M:
            return &state.columns[2].m;
         case kCol3R:
            return &state.columns[2].r;
         case kCol4S:
            return &state.columns[3].s;
         case kCol4M:
            return &state.columns[3].m;
         case kCol4R:
            return &state.columns[3].r;
         case kCol5S:
            return &state.columns[4].s;
         case kCol5M:
            return &state.columns[4].m;
         case kCol5R:
            return &state.columns[4].r;
         case kCol6S:
            return &state.columns[5].s;
         case kCol6M:
            return &state.columns[5].m;
         case kCol6R:
            return &state.columns[5].r;
         case kCol7S:
            return &state.columns[6].s;
         case kCol7M:
            return &state.columns[6].m;
         case kCol7R:
            return &state.columns[6].r;
         case kCol8S:
            return &state.columns[7].s;
         case kCol8M:
            return &state.columns[7].m;
         case kCol8R:
            return &state.columns[7].r;
         default:
            return nullptr;
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
         if (packet->length != 3) {
            continue;
         }
         kontroller->update(packet->data[1], packet->data[2]);
         packet = MIDIPacketNext(packet);
      }
   }

   MIDIEndpointRef findEndpoint() {
      ItemCount deviceCount = MIDIGetNumberOfDevices();

      for (ItemCount i = 0; i < deviceCount; ++i) {
         MIDIDeviceRef device = MIDIGetDevice(i);

         SInt32 offline = 0;
         MIDIObjectGetIntegerProperty(device, kMIDIPropertyOffline, &offline);
         if (offline) {
            continue;
         }

         CFStringRef name = nullptr;
         OSStatus nameResult = MIDIObjectGetStringProperty(device, kMIDIPropertyName, &name);
         if (nameResult != noErr ) {
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

         return MIDIEntityGetSource(entity, 0);
      }

      return 0;
   }
} // namespace

Kontroller::Kontroller() {
   MIDIEndpointRef endpoint = findEndpoint();
   if (!endpoint) {
      return;
   }

   OSStatus clientResult = MIDIClientCreate(CFSTR("Kontroller client"), nullptr, nullptr, &client);
   if (clientResult != noErr) {
      return;
   }

   OSStatus portResult = MIDIInputPortCreate(client, CFSTR("Kontroller port"), midiInputCallback, this, &port);
   if (portResult != noErr) {
      return;
   }

   MIDIPortConnectSource(port, endpoint, nullptr);
}

Kontroller::~Kontroller() {
   if (client) {
      MIDIClientDispose(client); // Closes the port as well
   }

   port = 0;
   client = 0;
}

KontrolState Kontroller::getState() const {
   std::lock_guard<std::mutex> lock(mutex);
   return state;
}

void Kontroller::update(unsigned char id, unsigned char value) {
   std::lock_guard<std::mutex> lock(mutex);

   float *floatVal = getFloatVal(state, id);
   if (floatVal) {
      *floatVal = value / 127.0f;
   } else {
      bool *boolVal = getBoolVal(state, id);
      if (boolVal) {
         *boolVal = value != 0;
      }
   }
}