#include "Kontroller/Kontroller.h"

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

const size_t kLEDModeOffset = 16;

ControlID idForLED(Kontroller::LED led) {
   switch (led) {
      case Kontroller::LED::kCycle: return kCycle;
      case Kontroller::LED::kRewind: return kRewind;
      case Kontroller::LED::kFastForward: return kFastForward;
      case Kontroller::LED::kStop: return kStop;
      case Kontroller::LED::kPlay: return kPlay;
      case Kontroller::LED::kRecord: return kRecord;
      case Kontroller::LED::kCol1S: return kCol1S;
      case Kontroller::LED::kCol1M: return kCol1M;
      case Kontroller::LED::kCol1R: return kCol1R;
      case Kontroller::LED::kCol2S: return kCol2S;
      case Kontroller::LED::kCol2M: return kCol2M;
      case Kontroller::LED::kCol2R: return kCol2R;
      case Kontroller::LED::kCol3S: return kCol3S;
      case Kontroller::LED::kCol3M: return kCol3M;
      case Kontroller::LED::kCol3R: return kCol3R;
      case Kontroller::LED::kCol4S: return kCol4S;
      case Kontroller::LED::kCol4M: return kCol4M;
      case Kontroller::LED::kCol4R: return kCol4R;
      case Kontroller::LED::kCol5S: return kCol5S;
      case Kontroller::LED::kCol5M: return kCol5M;
      case Kontroller::LED::kCol5R: return kCol5R;
      case Kontroller::LED::kCol6S: return kCol6S;
      case Kontroller::LED::kCol6M: return kCol6M;
      case Kontroller::LED::kCol6R: return kCol6R;
      case Kontroller::LED::kCol7S: return kCol7S;
      case Kontroller::LED::kCol7M: return kCol7M;
      case Kontroller::LED::kCol7R: return kCol7R;
      case Kontroller::LED::kCol8S: return kCol8S;
      case Kontroller::LED::kCol8M: return kCol8M;
      case Kontroller::LED::kCol8R: return kCol8R;
      default: KONTROLLER_ASSERT(false); return kTrackLeft;
   }
}

float* getFloatVal(Kontroller::State &state, uint8_t id) {
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

bool* getBoolVal(Kontroller::State &state, uint8_t id) {
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

} // namespace

Kontroller::Kontroller()
   : communicator(new Communicator(this)) {
}

void Kontroller::update(uint8_t id, uint8_t value) {
   std::lock_guard<std::mutex> lock(mutex);

   float *floatVal = getFloatVal(state, id);
   if (floatVal) {
      *floatVal = value / 127.0f;
   } else {
      bool *boolVal = getBoolVal(state, id);
      KONTROLLER_ASSERT(boolVal); // If it isn't a dial or slider, it should be a button

      *boolVal = value != 0;
   }
}

Kontroller::State Kontroller::getState() const {
   std::lock_guard<std::mutex> lock(mutex);
   return state;
}

void Kontroller::enableLEDControl(bool enable) {
   communicator->initializeMessage();

   communicator->appendToMessage(kStartSysex);
   communicator->appendToMessage(kSecondSysex);
   communicator->appendToMessage(kStartSysex);

   if (enable) {
      constexpr size_t size = kMainSysex.size();
      std::array<uint8_t, size> enableSysex(kMainSysex);
      enableSysex[kLEDModeOffset] = 0x01;
      communicator->appendToMessage(enableSysex);
   } else {
      communicator->appendToMessage(kMainSysex);
   }

   communicator->appendToMessage(kStartSysex);
   communicator->appendToMessage(kEndSysex);

   communicator->finalizeMessage();
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on) {
   std::array<uint8_t, 3> sendData;
   sendData[0] = 0xB0;
   sendData[1] = idForLED(led);
   sendData[2] = on ? 0x7F : 0x00;

   communicator->initializeMessage();

   communicator->appendToMessage(sendData);

   communicator->finalizeMessage();
}
