#include "Kontroller/Kontroller.h"
#include "Communicator.h"

namespace
{
   enum class ControlID : uint8_t
   {
      TrackPrevious = 0x3A,
      TrackNext = 0x3B,
      Cycle = 0x2E,
      MarkerSet = 0x3C,
      MarkerPrevious = 0x3D,
      MarkerNext = 0x3E,
      Rewind = 0x2B,
      FastForward = 0x2C,
      Stop = 0x2A,
      Play = 0x29,
      Record = 0x2D,
      Group1Dial = 0x10,
      Group1Slider = 0x00,
      Group1Solo = 0x20,
      Group1Mute = 0x30,
      Group1Record = 0x40,
      Group2Dial = Group1Dial + 1,
      Group2Slider = Group1Slider + 1,
      Group2Solo = Group1Solo + 1,
      Group2Mute = Group1Mute + 1,
      Group2Record = Group1Record + 1,
      Group3Dial = Group2Dial + 1,
      Group3Slider = Group2Slider + 1,
      Group3Solo = Group2Solo + 1,
      Group3Mute = Group2Mute + 1,
      Group3Record = Group2Record + 1,
      Group4Dial = Group3Dial + 1,
      Group4Slider = Group3Slider + 1,
      Group4Solo = Group3Solo + 1,
      Group4Mute = Group3Mute + 1,
      Group4Record = Group3Record + 1,
      Group5Dial = Group4Dial + 1,
      Group5Slider = Group4Slider + 1,
      Group5Solo = Group4Solo + 1,
      Group5Mute = Group4Mute + 1,
      Group5Record = Group4Record + 1,
      Group6Dial = Group5Dial + 1,
      Group6Slider = Group5Slider + 1,
      Group6Solo = Group5Solo + 1,
      Group6Mute = Group5Mute + 1,
      Group6Record = Group5Record + 1,
      Group7Dial = Group6Dial + 1,
      Group7Slider = Group6Slider + 1,
      Group7Solo = Group6Solo + 1,
      Group7Mute = Group6Mute + 1,
      Group7Record = Group6Record + 1,
      Group8Dial = Group7Dial + 1,
      Group8Slider = Group7Slider + 1,
      Group8Solo = Group7Solo + 1,
      Group8Mute = Group7Mute + 1,
      Group8Record = Group7Record + 1,
   };

   const std::array<uint8_t, 6> kStartSysex
   {{
      0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7
   }};

   const std::array<uint8_t, 402> kMainSysex
   {{
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

   const std::array<uint8_t, 11> kSecondSysex
   {{
      0xF0, 0x42, 0x40, 0x00, 0x01, 0x13, 0x00, 0x1F, 0x12, 0x00, 0xF7
   }};

   const std::array<uint8_t, 11> kEndSysex
   {{
      0xF0, 0x42, 0x40, 0x00, 0x01, 0x13, 0x00, 0x1F, 0x11, 0x00, 0xF7
   }};

   const std::size_t kLEDModeOffset = 16;

   ControlID idForLED(Kontroller::LED led)
   {
      switch (led)
      {
      case Kontroller::LED::Cycle: return ControlID::Cycle;
      case Kontroller::LED::Rewind: return ControlID::Rewind;
      case Kontroller::LED::FastForward: return ControlID::FastForward;
      case Kontroller::LED::Stop: return ControlID::Stop;
      case Kontroller::LED::Play: return ControlID::Play;
      case Kontroller::LED::Record: return ControlID::Record;
      case Kontroller::LED::Group1Solo: return ControlID::Group1Solo;
      case Kontroller::LED::Group1Mute: return ControlID::Group1Mute;
      case Kontroller::LED::Group1Record: return ControlID::Group1Record;
      case Kontroller::LED::Group2Solo: return ControlID::Group2Solo;
      case Kontroller::LED::Group2Mute: return ControlID::Group2Mute;
      case Kontroller::LED::Group2Record: return ControlID::Group2Record;
      case Kontroller::LED::Group3Solo: return ControlID::Group3Solo;
      case Kontroller::LED::Group3Mute: return ControlID::Group3Mute;
      case Kontroller::LED::Group3Record: return ControlID::Group3Record;
      case Kontroller::LED::Group4Solo: return ControlID::Group4Solo;
      case Kontroller::LED::Group4Mute: return ControlID::Group4Mute;
      case Kontroller::LED::Group4Record: return ControlID::Group4Record;
      case Kontroller::LED::Group5Solo: return ControlID::Group5Solo;
      case Kontroller::LED::Group5Mute: return ControlID::Group5Mute;
      case Kontroller::LED::Group5Record: return ControlID::Group5Record;
      case Kontroller::LED::Group6Solo: return ControlID::Group6Solo;
      case Kontroller::LED::Group6Mute: return ControlID::Group6Mute;
      case Kontroller::LED::Group6Record: return ControlID::Group6Record;
      case Kontroller::LED::Group7Solo: return ControlID::Group7Solo;
      case Kontroller::LED::Group7Mute: return ControlID::Group7Mute;
      case Kontroller::LED::Group7Record: return ControlID::Group7Record;
      case Kontroller::LED::Group8Solo: return ControlID::Group8Solo;
      case Kontroller::LED::Group8Mute: return ControlID::Group8Mute;
      case Kontroller::LED::Group8Record: return ControlID::Group8Record;
      default: return ControlID::TrackPrevious;
      }
   }

   Kontroller::Button buttonById(uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::TrackPrevious: return Kontroller::Button::TrackPrevious;
      case ControlID::TrackNext: return Kontroller::Button::TrackNext;
      case ControlID::Cycle: return Kontroller::Button::Cycle;
      case ControlID::MarkerSet: return Kontroller::Button::MarkerSet;
      case ControlID::MarkerPrevious: return Kontroller::Button::MarkerPrevious;
      case ControlID::MarkerNext: return Kontroller::Button::MarkerNext;
      case ControlID::Rewind: return Kontroller::Button::Rewind;
      case ControlID::FastForward: return Kontroller::Button::FastForward;
      case ControlID::Stop: return Kontroller::Button::Stop;
      case ControlID::Play: return Kontroller::Button::Play;
      case ControlID::Record: return Kontroller::Button::Record;
      case ControlID::Group1Solo: return Kontroller::Button::Group1Solo;
      case ControlID::Group1Mute: return Kontroller::Button::Group1Mute;
      case ControlID::Group1Record: return Kontroller::Button::Group1Record;
      case ControlID::Group2Solo: return Kontroller::Button::Group2Solo;
      case ControlID::Group2Mute: return Kontroller::Button::Group2Mute;
      case ControlID::Group2Record: return Kontroller::Button::Group2Record;
      case ControlID::Group3Solo: return Kontroller::Button::Group3Solo;
      case ControlID::Group3Mute: return Kontroller::Button::Group3Mute;
      case ControlID::Group3Record: return Kontroller::Button::Group3Record;
      case ControlID::Group4Solo: return Kontroller::Button::Group4Solo;
      case ControlID::Group4Mute: return Kontroller::Button::Group4Mute;
      case ControlID::Group4Record: return Kontroller::Button::Group4Record;
      case ControlID::Group5Solo: return Kontroller::Button::Group5Solo;
      case ControlID::Group5Mute: return Kontroller::Button::Group5Mute;
      case ControlID::Group5Record: return Kontroller::Button::Group5Record;
      case ControlID::Group6Solo: return Kontroller::Button::Group6Solo;
      case ControlID::Group6Mute: return Kontroller::Button::Group6Mute;
      case ControlID::Group6Record: return Kontroller::Button::Group6Record;
      case ControlID::Group7Solo: return Kontroller::Button::Group7Solo;
      case ControlID::Group7Mute: return Kontroller::Button::Group7Mute;
      case ControlID::Group7Record: return Kontroller::Button::Group7Record;
      case ControlID::Group8Solo: return Kontroller::Button::Group8Solo;
      case ControlID::Group8Mute: return Kontroller::Button::Group8Mute;
      case ControlID::Group8Record: return Kontroller::Button::Group8Record;
      default: return Kontroller::Button::None;
      }
   }

   Kontroller::Dial dialById(uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::Group1Dial: return Kontroller::Dial::Group1;
      case ControlID::Group2Dial: return Kontroller::Dial::Group2;
      case ControlID::Group3Dial: return Kontroller::Dial::Group3;
      case ControlID::Group4Dial: return Kontroller::Dial::Group4;
      case ControlID::Group5Dial: return Kontroller::Dial::Group5;
      case ControlID::Group6Dial: return Kontroller::Dial::Group6;
      case ControlID::Group7Dial: return Kontroller::Dial::Group7;
      case ControlID::Group8Dial: return Kontroller::Dial::Group8;
      default: return Kontroller::Dial::None;
      }
   }

   Kontroller::Slider sliderById(uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::Group1Slider: return Kontroller::Slider::Group1;
      case ControlID::Group2Slider: return Kontroller::Slider::Group2;
      case ControlID::Group3Slider: return Kontroller::Slider::Group3;
      case ControlID::Group4Slider: return Kontroller::Slider::Group4;
      case ControlID::Group5Slider: return Kontroller::Slider::Group5;
      case ControlID::Group6Slider: return Kontroller::Slider::Group6;
      case ControlID::Group7Slider: return Kontroller::Slider::Group7;
      case ControlID::Group8Slider: return Kontroller::Slider::Group8;
      default: return Kontroller::Slider::None;
      }
   }

   float* getDialVal(Kontroller::State& state, uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::Group1Dial: return &state.groups[0].dial;
      case ControlID::Group2Dial: return &state.groups[1].dial;
      case ControlID::Group3Dial: return &state.groups[2].dial;
      case ControlID::Group4Dial: return &state.groups[3].dial;
      case ControlID::Group5Dial: return &state.groups[4].dial;
      case ControlID::Group6Dial: return &state.groups[5].dial;
      case ControlID::Group7Dial: return &state.groups[6].dial;
      case ControlID::Group8Dial: return &state.groups[7].dial;
      default: return nullptr;
      }
   }

   float* getSliderVal(Kontroller::State& state, uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::Group1Slider: return &state.groups[0].slider;
      case ControlID::Group2Slider: return &state.groups[1].slider;
      case ControlID::Group3Slider: return &state.groups[2].slider;
      case ControlID::Group4Slider: return &state.groups[3].slider;
      case ControlID::Group5Slider: return &state.groups[4].slider;
      case ControlID::Group6Slider: return &state.groups[5].slider;
      case ControlID::Group7Slider: return &state.groups[6].slider;
      case ControlID::Group8Slider: return &state.groups[7].slider;
      default: return nullptr;
      }
   }

   bool* getButtonVal(Kontroller::State& state, uint8_t id)
   {
      ControlID controlId = static_cast<ControlID>(id);
      switch (controlId)
      {
      case ControlID::TrackPrevious: return &state.trackPrevious;
      case ControlID::TrackNext: return &state.trackNext;
      case ControlID::Cycle: return &state.cycle;
      case ControlID::MarkerSet: return &state.markerSet;
      case ControlID::MarkerPrevious: return &state.markerPrevious;
      case ControlID::MarkerNext: return &state.markerNext;
      case ControlID::Rewind: return &state.rewind;
      case ControlID::FastForward: return &state.fastForward;
      case ControlID::Stop: return &state.stop;
      case ControlID::Play: return &state.play;
      case ControlID::Record: return &state.record;
      case ControlID::Group1Solo: return &state.groups[0].solo;
      case ControlID::Group1Mute: return &state.groups[0].mute;
      case ControlID::Group1Record: return &state.groups[0].record;
      case ControlID::Group2Solo: return &state.groups[1].solo;
      case ControlID::Group2Mute: return &state.groups[1].mute;
      case ControlID::Group2Record: return &state.groups[1].record;
      case ControlID::Group3Solo: return &state.groups[2].solo;
      case ControlID::Group3Mute: return &state.groups[2].mute;
      case ControlID::Group3Record: return &state.groups[2].record;
      case ControlID::Group4Solo: return &state.groups[3].solo;
      case ControlID::Group4Mute: return &state.groups[3].mute;
      case ControlID::Group4Record: return &state.groups[3].record;
      case ControlID::Group5Solo: return &state.groups[4].solo;
      case ControlID::Group5Mute: return &state.groups[4].mute;
      case ControlID::Group5Record: return &state.groups[4].record;
      case ControlID::Group6Solo: return &state.groups[5].solo;
      case ControlID::Group6Mute: return &state.groups[5].mute;
      case ControlID::Group6Record: return &state.groups[5].record;
      case ControlID::Group7Solo: return &state.groups[6].solo;
      case ControlID::Group7Mute: return &state.groups[6].mute;
      case ControlID::Group7Record: return &state.groups[6].record;
      case ControlID::Group8Solo: return &state.groups[7].solo;
      case ControlID::Group8Mute: return &state.groups[7].mute;
      case ControlID::Group8Record: return &state.groups[7].record;
      default: return nullptr;
      }
   }

   bool processControlCommand(Kontroller::Communicator& communicator, bool enable)
   {
      bool success = communicator.initializeMessage();

      success = success && communicator.appendToMessage(kStartSysex);
      success = success && communicator.appendToMessage(kSecondSysex);

      success = success && communicator.appendToMessage(kStartSysex);
      if (enable)
      {
         auto enableSysex = kMainSysex;
         enableSysex[kLEDModeOffset] = 0x01;
         success = success && communicator.appendToMessage(enableSysex);
      }
      else
      {
         success = success && communicator.appendToMessage(kMainSysex);
      }

      success = success && communicator.appendToMessage(kStartSysex);
      success = success && communicator.appendToMessage(kEndSysex);

      success = success && communicator.finalizeMessage();

      return success;
   }

   bool processLEDCommand(Kontroller::Communicator& communicator, Kontroller::LED led, bool enable)
   {
      std::array<uint8_t, 3> sendData;
      sendData[0] = 0xB0;
      sendData[1] = static_cast<uint8_t>(idForLED(led));
      sendData[2] = enable ? 0x7F : 0x00;

      bool success = communicator.initializeMessage();
      success = success && communicator.appendToMessage(sendData);
      success = success && communicator.finalizeMessage();

      return success;
   }
}

Kontroller::Kontroller()
   : thread([this]{ threadRun(); })
{
}

Kontroller::~Kontroller()
{
   {
      std::unique_lock<std::mutex> lock(eventMutex);
      shuttingDown.store(true);
   }
   cv.notify_all();

   thread.join();
}

Kontroller::State Kontroller::getState() const
{
   std::lock_guard<std::mutex> lock(stateMutex);
   return state;
}

void Kontroller::enableLEDControl(bool enable)
{
   MidiCommand command;
   command.type = MidiCommand::Type::Control;
   command.value = enable;

   commandQueue.enqueue(command);

   {
      std::unique_lock<std::mutex> lock(eventMutex);
      eventPending.store(true);
   }
   cv.notify_all();
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on)
{
   MidiCommand command;
   command.type = MidiCommand::Type::LED;
   command.led = led;
   command.value = on;

   commandQueue.enqueue(command);

   {
      std::unique_lock<std::mutex> lock(eventMutex);
      eventPending.store(true);
   }
   cv.notify_all();
}

void Kontroller::setButtonCallback(ButtonCallback callback)
{
   std::lock_guard<std::recursive_mutex> lock(callbackMutex);
   buttonCallback = std::move(callback);
}

void Kontroller::clearButtonCallback()
{
   setButtonCallback({});
}

void Kontroller::setDialCallback(DialCallback callback)
{
   std::lock_guard<std::recursive_mutex> lock(callbackMutex);
   dialCallback = std::move(callback);
}

void Kontroller::clearDialCallback()
{
   setDialCallback({});
}

void Kontroller::setSliderCallback(SliderCallback callback)
{
   std::lock_guard<std::recursive_mutex> lock(callbackMutex);
   sliderCallback = std::move(callback);
}

void Kontroller::clearSliderCallback()
{
   setSliderCallback({});
}

// static
Kontroller::State Kontroller::getOnlyNewButtons(const State& previous, const State& current)
{
   State onlyNew = current;

   for (std::size_t i = 0; i < onlyNew.groups.size(); ++i)
   {
      onlyNew.groups[i].solo = current.groups[i].solo && !previous.groups[i].solo;
      onlyNew.groups[i].mute = current.groups[i].mute && !previous.groups[i].mute;
      onlyNew.groups[i].record = current.groups[i].record && !previous.groups[i].record;
   }

   onlyNew.trackPrevious = current.trackPrevious && !previous.trackPrevious;
   onlyNew.trackNext = current.trackNext && !previous.trackNext;

   onlyNew.cycle = current.cycle && !previous.cycle;

   onlyNew.markerSet = current.markerSet && !previous.markerSet;
   onlyNew.markerPrevious = current.markerPrevious && !previous.markerPrevious;
   onlyNew.markerNext = current.markerNext && !previous.markerNext;

   onlyNew.rewind = current.rewind && !previous.rewind;
   onlyNew.fastForward = current.fastForward && !previous.fastForward;
   onlyNew.stop = current.stop && !previous.stop;
   onlyNew.play = current.play && !previous.play;
   onlyNew.record = current.record && !previous.record;

   return onlyNew;
}

// static
const char* const Kontroller::kDeviceName = "nanoKONTROL2";

void Kontroller::queueMessage(uint8_t id, uint8_t value)
{
   MidiMessage message;
   message.id = id;
   message.value = value;

   messageQueue.enqueue(message);

   {
      std::unique_lock<std::mutex> lock(eventMutex);
      eventPending.store(true);
   }
   cv.notify_all();
}

void Kontroller::threadRun()
{
   Communicator communicator(*this);
   bool wasConnected = false;

   bool shouldExit = false;
   while (!shouldExit)
   {
      {
         // Wait until there's something to do (with a timeout for handling (dis)connection)
         std::unique_lock<std::mutex> lock(eventMutex);
         cv.wait_for(lock, std::chrono::milliseconds(1000), [this]
         {
            return shuttingDown.load() || eventPending.load();
         });

         eventPending.store(false);
      }

      // Only update whether we should exit here, so that we make sure to send any final commands before shutting down
      shouldExit = shuttingDown;

      // Read any pending messages
      MidiMessage message;
      while (messageQueue.try_dequeue(message))
      {
         processMessage(message);
      }

      // Poll the communicator (to check for disconnection)
      communicator.poll();

      // Check if we're still connected
      bool isConnected = communicator.isConnected();
      if (!isConnected && !shouldExit)
      {
         isConnected = communicator.connect();
      }

      // Update connection status
      if (isConnected != wasConnected)
      {
         communicatorConnected.store(isConnected);
      }
      wasConnected = isConnected;

      // Send any pending commands (if connected)
      if (isConnected)
      {
         MidiCommand command;
         while (commandQueue.try_dequeue(command))
         {
            bool success = false;
            switch (command.type)
            {
            case MidiCommand::Type::Control:
               success = processControlCommand(communicator, command.value);
               break;
            case MidiCommand::Type::LED:
               success = processLEDCommand(communicator, command.led, command.value);
               break;
            default:
               break;
            }

            if (!success)
            {
               break;
            }
         }
      }
   }
}

void Kontroller::processMessage(MidiMessage message)
{
   bool isButton = false;
   bool isDial = false;
   bool isSlider = false;

   bool buttonValue = false;
   float dialValue = 0.0f;
   float sliderValue = 0.0f;

   {
      std::lock_guard<std::mutex> lock(stateMutex);

      if (bool* buttonVal = getButtonVal(state, message.id))
      {
         *buttonVal = message.value != 0;

         isButton = true;
         buttonValue = *buttonVal;
      }
      else if (float* dialVal = getDialVal(state, message.id))
      {
         *dialVal = message.value / 127.0f;

         isDial = true;
         dialValue = *dialVal;
      }
      else if (float* sliderVal = getSliderVal(state, message.id))
      {
         *sliderVal = message.value / 127.0f;

         isSlider = true;
         sliderValue = *sliderVal;
      }
   }

   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);

      if (isButton && buttonCallback)
      {
         buttonCallback(buttonById(message.id), buttonValue);
      }
      else if (isDial && dialCallback)
      {
         dialCallback(dialById(message.id), dialValue);
      }
      else if (isSlider && sliderCallback)
      {
         sliderCallback(sliderById(message.id), sliderValue);
      }
   }
}
