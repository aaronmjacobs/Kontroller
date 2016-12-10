#include "Kontroller/Kontroller.h"
#include "Communicator.h"

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
   kGroup1Dial = 0x10,
   kGroup1Slider = 0x00,
   kGroup1Solo = 0x20,
   kGroup1Mute = 0x30,
   kGroup1Record = 0x40,
   kGroup2Dial = kGroup1Dial + 1,
   kGroup2Slider = kGroup1Slider + 1,
   kGroup2Solo = kGroup1Solo + 1,
   kGroup2Mute = kGroup1Mute + 1,
   kGroup2Record = kGroup1Record + 1,
   kGroup3Dial = kGroup2Dial + 1,
   kGroup3Slider = kGroup2Slider + 1,
   kGroup3Solo = kGroup2Solo + 1,
   kGroup3Mute = kGroup2Mute + 1,
   kGroup3Record = kGroup2Record + 1,
   kGroup4Dial = kGroup3Dial + 1,
   kGroup4Slider = kGroup3Slider + 1,
   kGroup4Solo = kGroup3Solo + 1,
   kGroup4Mute = kGroup3Mute + 1,
   kGroup4Record = kGroup3Record + 1,
   kGroup5Dial = kGroup4Dial + 1,
   kGroup5Slider = kGroup4Slider + 1,
   kGroup5Solo = kGroup4Solo + 1,
   kGroup5Mute = kGroup4Mute + 1,
   kGroup5Record = kGroup4Record + 1,
   kGroup6Dial = kGroup5Dial + 1,
   kGroup6Slider = kGroup5Slider + 1,
   kGroup6Solo = kGroup5Solo + 1,
   kGroup6Mute = kGroup5Mute + 1,
   kGroup6Record = kGroup5Record + 1,
   kGroup7Dial = kGroup6Dial + 1,
   kGroup7Slider = kGroup6Slider + 1,
   kGroup7Solo = kGroup6Solo + 1,
   kGroup7Mute = kGroup6Mute + 1,
   kGroup7Record = kGroup6Record + 1,
   kGroup8Dial = kGroup7Dial + 1,
   kGroup8Slider = kGroup7Slider + 1,
   kGroup8Solo = kGroup7Solo + 1,
   kGroup8Mute = kGroup7Mute + 1,
   kGroup8Record = kGroup7Record + 1,
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

const std::chrono::milliseconds kConnectSleepTime = std::chrono::milliseconds(500);
const std::chrono::milliseconds kMaxWaitTime = std::chrono::milliseconds(500);

ControlID idForLED(Kontroller::LED led) {
   switch (led) {
      case Kontroller::LED::kCycle: return kCycle;
      case Kontroller::LED::kRewind: return kRewind;
      case Kontroller::LED::kFastForward: return kFastForward;
      case Kontroller::LED::kStop: return kStop;
      case Kontroller::LED::kPlay: return kPlay;
      case Kontroller::LED::kRecord: return kRecord;
      case Kontroller::LED::kGroup1Solo: return kGroup1Solo;
      case Kontroller::LED::kGroup1Mute: return kGroup1Mute;
      case Kontroller::LED::kGroup1Record: return kGroup1Record;
      case Kontroller::LED::kGroup2Solo: return kGroup2Solo;
      case Kontroller::LED::kGroup2Mute: return kGroup2Mute;
      case Kontroller::LED::kGroup2Record: return kGroup2Record;
      case Kontroller::LED::kGroup3Solo: return kGroup3Solo;
      case Kontroller::LED::kGroup3Mute: return kGroup3Mute;
      case Kontroller::LED::kGroup3Record: return kGroup3Record;
      case Kontroller::LED::kGroup4Solo: return kGroup4Solo;
      case Kontroller::LED::kGroup4Mute: return kGroup4Mute;
      case Kontroller::LED::kGroup4Record: return kGroup4Record;
      case Kontroller::LED::kGroup5Solo: return kGroup5Solo;
      case Kontroller::LED::kGroup5Mute: return kGroup5Mute;
      case Kontroller::LED::kGroup5Record: return kGroup5Record;
      case Kontroller::LED::kGroup6Solo: return kGroup6Solo;
      case Kontroller::LED::kGroup6Mute: return kGroup6Mute;
      case Kontroller::LED::kGroup6Record: return kGroup6Record;
      case Kontroller::LED::kGroup7Solo: return kGroup7Solo;
      case Kontroller::LED::kGroup7Mute: return kGroup7Mute;
      case Kontroller::LED::kGroup7Record: return kGroup7Record;
      case Kontroller::LED::kGroup8Solo: return kGroup8Solo;
      case Kontroller::LED::kGroup8Mute: return kGroup8Mute;
      case Kontroller::LED::kGroup8Record: return kGroup8Record;
      default: return kTrackLeft;
   }
}

float* getDialVal(Kontroller::State &state, uint8_t id) {
   switch (id) {
   case kGroup1Dial: return &state.groups[0].dial;
   case kGroup2Dial: return &state.groups[1].dial;
   case kGroup3Dial: return &state.groups[2].dial;
   case kGroup4Dial: return &state.groups[3].dial;
   case kGroup5Dial: return &state.groups[4].dial;
   case kGroup6Dial: return &state.groups[5].dial;
   case kGroup7Dial: return &state.groups[6].dial;
   case kGroup8Dial: return &state.groups[7].dial;
   default: return nullptr;
   }
}

float* getSliderVal(Kontroller::State &state, uint8_t id) {
   switch (id) {
      case kGroup1Slider: return &state.groups[0].slider;
      case kGroup2Slider: return &state.groups[1].slider;
      case kGroup3Slider: return &state.groups[2].slider;
      case kGroup4Slider: return &state.groups[3].slider;
      case kGroup5Slider: return &state.groups[4].slider;
      case kGroup6Slider: return &state.groups[5].slider;
      case kGroup7Slider: return &state.groups[6].slider;
      case kGroup8Slider: return &state.groups[7].slider;
      default: return nullptr;
   }
}

bool* getButtonVal(Kontroller::State &state, uint8_t id) {
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
      case kGroup1Solo: return &state.groups[0].solo;
      case kGroup1Mute: return &state.groups[0].mute;
      case kGroup1Record: return &state.groups[0].record;
      case kGroup2Solo: return &state.groups[1].solo;
      case kGroup2Mute: return &state.groups[1].mute;
      case kGroup2Record: return &state.groups[1].record;
      case kGroup3Solo: return &state.groups[2].solo;
      case kGroup3Mute: return &state.groups[2].mute;
      case kGroup3Record: return &state.groups[2].record;
      case kGroup4Solo: return &state.groups[3].solo;
      case kGroup4Mute: return &state.groups[3].mute;
      case kGroup4Record: return &state.groups[3].record;
      case kGroup5Solo: return &state.groups[4].solo;
      case kGroup5Mute: return &state.groups[4].mute;
      case kGroup5Record: return &state.groups[4].record;
      case kGroup6Solo: return &state.groups[5].solo;
      case kGroup6Mute: return &state.groups[5].mute;
      case kGroup6Record: return &state.groups[5].record;
      case kGroup7Solo: return &state.groups[6].solo;
      case kGroup7Mute: return &state.groups[6].mute;
      case kGroup7Record: return &state.groups[6].record;
      case kGroup8Solo: return &state.groups[7].solo;
      case kGroup8Mute: return &state.groups[7].mute;
      case kGroup8Record: return &state.groups[7].record;
      default: return nullptr;
   }
}

Kontroller::Button buttonById(uint8_t id) {
   switch (id) {
      case kTrackLeft: return Kontroller::Button::kTrackLeft;
      case kTrackRight: return Kontroller::Button::kTrackRight;
      case kCycle: return Kontroller::Button::kCycle;
      case kMarkerSet: return Kontroller::Button::kMarkerSet;
      case kMarkerLeft: return Kontroller::Button::kMarkerLeft;
      case kMarkerRight: return Kontroller::Button::kMarkerRight;
      case kRewind: return Kontroller::Button::kRewind;
      case kFastForward: return Kontroller::Button::kFastForward;
      case kStop: return Kontroller::Button::kStop;
      case kPlay: return Kontroller::Button::kPlay;
      case kRecord: return Kontroller::Button::kRecord;
      case kGroup1Solo: return Kontroller::Button::kGroup1Solo;
      case kGroup1Mute: return Kontroller::Button::kGroup1Mute;
      case kGroup1Record: return Kontroller::Button::kGroup1Record;
      case kGroup2Solo: return Kontroller::Button::kGroup2Solo;
      case kGroup2Mute: return Kontroller::Button::kGroup2Mute;
      case kGroup2Record: return Kontroller::Button::kGroup2Record;
      case kGroup3Solo: return Kontroller::Button::kGroup3Solo;
      case kGroup3Mute: return Kontroller::Button::kGroup3Mute;
      case kGroup3Record: return Kontroller::Button::kGroup3Record;
      case kGroup4Solo: return Kontroller::Button::kGroup4Solo;
      case kGroup4Mute: return Kontroller::Button::kGroup4Mute;
      case kGroup4Record: return Kontroller::Button::kGroup4Record;
      case kGroup5Solo: return Kontroller::Button::kGroup5Solo;
      case kGroup5Mute: return Kontroller::Button::kGroup5Mute;
      case kGroup5Record: return Kontroller::Button::kGroup5Record;
      case kGroup6Solo: return Kontroller::Button::kGroup6Solo;
      case kGroup6Mute: return Kontroller::Button::kGroup6Mute;
      case kGroup6Record: return Kontroller::Button::kGroup6Record;
      case kGroup7Solo: return Kontroller::Button::kGroup7Solo;
      case kGroup7Mute: return Kontroller::Button::kGroup7Mute;
      case kGroup7Record: return Kontroller::Button::kGroup7Record;
      case kGroup8Solo: return Kontroller::Button::kGroup8Solo;
      case kGroup8Mute: return Kontroller::Button::kGroup8Mute;
      case kGroup8Record: return Kontroller::Button::kGroup8Record;
      default: return Kontroller::Button::kNone;
   }
}

Kontroller::Dial dialById(uint8_t id) {
   switch (id) {
      case kGroup1Dial: return Kontroller::Dial::kGroup1;
      case kGroup2Dial: return Kontroller::Dial::kGroup2;
      case kGroup3Dial: return Kontroller::Dial::kGroup3;
      case kGroup4Dial: return Kontroller::Dial::kGroup4;
      case kGroup5Dial: return Kontroller::Dial::kGroup5;
      case kGroup6Dial: return Kontroller::Dial::kGroup6;
      case kGroup7Dial: return Kontroller::Dial::kGroup7;
      case kGroup8Dial: return Kontroller::Dial::kGroup8;
      default: return Kontroller::Dial::kNone;
   }
}

Kontroller::Slider sliderById(uint8_t id) {
   switch (id) {
   case kGroup1Slider: return Kontroller::Slider::kGroup1;
   case kGroup2Slider: return Kontroller::Slider::kGroup2;
   case kGroup3Slider: return Kontroller::Slider::kGroup3;
   case kGroup4Slider: return Kontroller::Slider::kGroup4;
   case kGroup5Slider: return Kontroller::Slider::kGroup5;
   case kGroup6Slider: return Kontroller::Slider::kGroup6;
   case kGroup7Slider: return Kontroller::Slider::kGroup7;
   case kGroup8Slider: return Kontroller::Slider::kGroup8;
   default: return Kontroller::Slider::kNone;
   }
}

} // namespace

// static
const char* const Kontroller::kDeviceName = "nanoKONTROL2";

Kontroller::Kontroller()
   : shuttingDown(false), communicator(std::make_unique<Communicator>(this)), thread([this]{ threadRun(); }) {
}

// This must be defined here so that the default deleter can be used for Kontroller::Communicator (which is an
// incomplete type in Kontroller.h)
Kontroller::~Kontroller() {
   shuttingDown = true;
   cv.notify_all();
   thread.join();
}

bool Kontroller::isConnected() const {
   return communicator->isConnected();
}

void Kontroller::enableLEDControl(bool enable) {
   MidiCommand command{};
   command.type = MidiCommand::Type::kControl;
   command.value = enable;

   commandQueue.enqueue(command);
   cv.notify_all();
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on) {
   MidiCommand command{};
   command.type = MidiCommand::Type::kLED;
   command.led = led;
   command.value = on;

   commandQueue.enqueue(command);
   cv.notify_all();
}

void Kontroller::update(uint8_t id, uint8_t value) {
   MidiMessage message;
   message.id = id;
   message.value = value;

   messageQueue.enqueue(message);
   cv.notify_all();
}

void Kontroller::threadRun() {
   std::mutex lifetimeMutex;
   std::unique_lock<std::mutex> lock(lifetimeMutex);

   bool shouldExit = false;
   while (!shouldExit) {
      // Wait until there's something to do
      cv.wait_for(lock, kMaxWaitTime, [this]{
         return messageQueue.peek() != nullptr || commandQueue.peek() != nullptr || !communicator->isConnected();
      });

      // Only update whether we should exit here, so that we make sure to send any final commands before shutting down
      shouldExit = shuttingDown;

      // Read any pending messages
      MidiMessage message;
      while (messageQueue.try_dequeue(message)) {
         processMessage(message);
      }

      // Poll the communicator (to check for disconnection)
      communicator->poll();

      // Check if we're still connected
      bool isConnected = communicator->isConnected();
      if (!isConnected && !shuttingDown) {
         isConnected = communicator->connect();
      }

      if (isConnected) {
         // Send any pending commands
         MidiCommand command;
         while (commandQueue.try_dequeue(command)) {
            processCommand(command);
         }
      } else if (!shuttingDown) {
         // Sleep a bit before trying to connect again
         std::this_thread::sleep_for(kConnectSleepTime);
      }
   }
}

void Kontroller::processMessage(MidiMessage message) {
   bool isButton = false;
   bool isDial = false;
   bool isSlider = false;

   bool buttonValue = false;
   float dialValue = 0.0f;
   float sliderValue = 0.0f;

   {
      std::lock_guard<std::mutex> lock(valueMutex);

      if (bool* buttonVal = getButtonVal(state, message.id)) {
         *buttonVal = message.value != 0;

         isButton = true;
         buttonValue = *buttonVal;
      }
      else if (float* dialVal = getDialVal(state, message.id)) {
         *dialVal = message.value / 127.0f;

         isDial = true;
         dialValue = *dialVal;
      }
      else if (float* sliderVal = getSliderVal(state, message.id)) {
         *sliderVal = message.value / 127.0f;

         isSlider = true;
         sliderValue = *sliderVal;
      }
   }

   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);

      if (isButton && buttonCallback) {
         buttonCallback(buttonById(message.id), buttonValue);
      } else if (isDial && dialCallback) {
         dialCallback(dialById(message.id), dialValue);
      } else if (isSlider && sliderCallback) {
         sliderCallback(sliderById(message.id), sliderValue);
      }
   }
}

void Kontroller::processCommand(MidiCommand command) {
   switch (command.type) {
      case MidiCommand::Type::kControl:
         processControlCommand(command.value);
         break;
      case MidiCommand::Type::kLED:
         processLEDCommand(command.led, command.value);
         break;
   }
}

void Kontroller::processControlCommand(bool enable) {
   bool success = communicator->initializeMessage();

   success = success && communicator->appendToMessage(kStartSysex);
   success = success && communicator->appendToMessage(kSecondSysex);

   success = success && communicator->appendToMessage(kStartSysex);
   if (enable) {
      constexpr size_t size = kMainSysex.size();
      std::array<uint8_t, size> enableSysex(kMainSysex);
      enableSysex[kLEDModeOffset] = 0x01;
      success = success && communicator->appendToMessage(enableSysex);
   } else {
      success = success && communicator->appendToMessage(kMainSysex);
   }

   success = success && communicator->appendToMessage(kStartSysex);
   success = success && communicator->appendToMessage(kEndSysex);

   success = success && communicator->finalizeMessage();

   // TODO Figure out how to handle failure (error state? callback?)
   //return success;
}

void Kontroller::processLEDCommand(LED led, bool enable) {
   std::array<uint8_t, 3> sendData;
   sendData[0] = 0xB0;
   sendData[1] = idForLED(led);
   sendData[2] = enable ? 0x7F : 0x00;

   bool success = communicator->initializeMessage();
   success = success && communicator->appendToMessage(sendData);
   success = success && communicator->finalizeMessage();

   // TODO Figure out how to handle failure (error state? callback?)
   //return success;
}
