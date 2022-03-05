#include "Kontroller/Device.h"
#include "Communicator.h"

#include <chrono>

namespace Kontroller
{
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

      ControlID idForLED(LED led)
      {
         switch (led)
         {
         case LED::Cycle: return ControlID::Cycle;
         case LED::Rewind: return ControlID::Rewind;
         case LED::FastForward: return ControlID::FastForward;
         case LED::Stop: return ControlID::Stop;
         case LED::Play: return ControlID::Play;
         case LED::Record: return ControlID::Record;
         case LED::Group1Solo: return ControlID::Group1Solo;
         case LED::Group1Mute: return ControlID::Group1Mute;
         case LED::Group1Record: return ControlID::Group1Record;
         case LED::Group2Solo: return ControlID::Group2Solo;
         case LED::Group2Mute: return ControlID::Group2Mute;
         case LED::Group2Record: return ControlID::Group2Record;
         case LED::Group3Solo: return ControlID::Group3Solo;
         case LED::Group3Mute: return ControlID::Group3Mute;
         case LED::Group3Record: return ControlID::Group3Record;
         case LED::Group4Solo: return ControlID::Group4Solo;
         case LED::Group4Mute: return ControlID::Group4Mute;
         case LED::Group4Record: return ControlID::Group4Record;
         case LED::Group5Solo: return ControlID::Group5Solo;
         case LED::Group5Mute: return ControlID::Group5Mute;
         case LED::Group5Record: return ControlID::Group5Record;
         case LED::Group6Solo: return ControlID::Group6Solo;
         case LED::Group6Mute: return ControlID::Group6Mute;
         case LED::Group6Record: return ControlID::Group6Record;
         case LED::Group7Solo: return ControlID::Group7Solo;
         case LED::Group7Mute: return ControlID::Group7Mute;
         case LED::Group7Record: return ControlID::Group7Record;
         case LED::Group8Solo: return ControlID::Group8Solo;
         case LED::Group8Mute: return ControlID::Group8Mute;
         case LED::Group8Record: return ControlID::Group8Record;
         default: return ControlID::TrackPrevious;
         }
      }

      Button buttonById(uint8_t id)
      {
         ControlID controlId = static_cast<ControlID>(id);
         switch (controlId)
         {
         case ControlID::TrackPrevious: return Button::TrackPrevious;
         case ControlID::TrackNext: return Button::TrackNext;
         case ControlID::Cycle: return Button::Cycle;
         case ControlID::MarkerSet: return Button::MarkerSet;
         case ControlID::MarkerPrevious: return Button::MarkerPrevious;
         case ControlID::MarkerNext: return Button::MarkerNext;
         case ControlID::Rewind: return Button::Rewind;
         case ControlID::FastForward: return Button::FastForward;
         case ControlID::Stop: return Button::Stop;
         case ControlID::Play: return Button::Play;
         case ControlID::Record: return Button::Record;
         case ControlID::Group1Solo: return Button::Group1Solo;
         case ControlID::Group1Mute: return Button::Group1Mute;
         case ControlID::Group1Record: return Button::Group1Record;
         case ControlID::Group2Solo: return Button::Group2Solo;
         case ControlID::Group2Mute: return Button::Group2Mute;
         case ControlID::Group2Record: return Button::Group2Record;
         case ControlID::Group3Solo: return Button::Group3Solo;
         case ControlID::Group3Mute: return Button::Group3Mute;
         case ControlID::Group3Record: return Button::Group3Record;
         case ControlID::Group4Solo: return Button::Group4Solo;
         case ControlID::Group4Mute: return Button::Group4Mute;
         case ControlID::Group4Record: return Button::Group4Record;
         case ControlID::Group5Solo: return Button::Group5Solo;
         case ControlID::Group5Mute: return Button::Group5Mute;
         case ControlID::Group5Record: return Button::Group5Record;
         case ControlID::Group6Solo: return Button::Group6Solo;
         case ControlID::Group6Mute: return Button::Group6Mute;
         case ControlID::Group6Record: return Button::Group6Record;
         case ControlID::Group7Solo: return Button::Group7Solo;
         case ControlID::Group7Mute: return Button::Group7Mute;
         case ControlID::Group7Record: return Button::Group7Record;
         case ControlID::Group8Solo: return Button::Group8Solo;
         case ControlID::Group8Mute: return Button::Group8Mute;
         case ControlID::Group8Record: return Button::Group8Record;
         default: return Button::None;
         }
      }

      Dial dialById(uint8_t id)
      {
         ControlID controlId = static_cast<ControlID>(id);
         switch (controlId)
         {
         case ControlID::Group1Dial: return Dial::Group1;
         case ControlID::Group2Dial: return Dial::Group2;
         case ControlID::Group3Dial: return Dial::Group3;
         case ControlID::Group4Dial: return Dial::Group4;
         case ControlID::Group5Dial: return Dial::Group5;
         case ControlID::Group6Dial: return Dial::Group6;
         case ControlID::Group7Dial: return Dial::Group7;
         case ControlID::Group8Dial: return Dial::Group8;
         default: return Dial::None;
         }
      }

      Slider sliderById(uint8_t id)
      {
         ControlID controlId = static_cast<ControlID>(id);
         switch (controlId)
         {
         case ControlID::Group1Slider: return Slider::Group1;
         case ControlID::Group2Slider: return Slider::Group2;
         case ControlID::Group3Slider: return Slider::Group3;
         case ControlID::Group4Slider: return Slider::Group4;
         case ControlID::Group5Slider: return Slider::Group5;
         case ControlID::Group6Slider: return Slider::Group6;
         case ControlID::Group7Slider: return Slider::Group7;
         case ControlID::Group8Slider: return Slider::Group8;
         default: return Slider::None;
         }
      }

      bool processControlCommand(Device::Communicator& communicator, bool enable)
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

      bool processLEDCommand(Device::Communicator& communicator, LED led, bool enable)
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

   Device::Device()
      : thread([this]{ threadRun(); })
   {
   }

   Device::~Device()
   {
      {
         std::lock_guard<std::mutex> lock(eventMutex);
         shuttingDown.store(true);
      }
      cv.notify_all();

      thread.join();
   }

   State Device::getState() const
   {
      std::lock_guard<std::mutex> lock(stateMutex);
      return state;
   }

   void Device::enableLEDControl(bool enable)
   {
      MidiCommand command;
      command.type = MidiCommand::Type::Control;
      command.value = enable;

      commandQueue.enqueue(command);

      {
         std::lock_guard<std::mutex> lock(eventMutex);
         eventPending.store(true);
      }
      cv.notify_all();
   }

   void Device::setLEDOn(LED led, bool on)
   {
      MidiCommand command;
      command.type = MidiCommand::Type::LED;
      command.led = led;
      command.value = on;

      commandQueue.enqueue(command);

      {
         std::lock_guard<std::mutex> lock(eventMutex);
         eventPending.store(true);
      }
      cv.notify_all();
   }

   void Device::setButtonCallback(ButtonCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      buttonCallback = std::move(callback);
   }

   void Device::clearButtonCallback()
   {
      setButtonCallback({});
   }

   void Device::setDialCallback(DialCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      dialCallback = std::move(callback);
   }

   void Device::clearDialCallback()
   {
      setDialCallback({});
   }

   void Device::setSliderCallback(SliderCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      sliderCallback = std::move(callback);
   }

   void Device::clearSliderCallback()
   {
      setSliderCallback({});
   }

   // static
   const char* const Device::kDeviceName = "nanoKONTROL2";

   void Device::queueMessage(uint8_t id, uint8_t value)
   {
      MidiMessage message;
      message.id = id;
      message.value = value;

      messageQueue.enqueue(message);

      {
         std::lock_guard<std::mutex> lock(eventMutex);
         eventPending.store(true);
      }
      cv.notify_all();
   }

   void Device::threadRun()
   {
      Communicator communicator(*this);
      bool wasConnected = false;
      auto lastPollTime = std::chrono::steady_clock::now();

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
         auto now = std::chrono::steady_clock::now();
         if (now - lastPollTime > std::chrono::milliseconds(100))
         {
            communicator.poll();
         }
         lastPollTime = now;

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
                  while (commandQueue.pop());
                  communicator.onConnectionLost();
                  break;
               }
            }
         }
      }
   }

   void Device::processMessage(MidiMessage message)
   {
      Button button = buttonById(message.id);
      Dial dial = dialById(message.id);
      Slider slider = sliderById(message.id);

      bool boolValue = message.value != 0;
      float floatValue = message.value / 127.0f;

      {
         std::lock_guard<std::mutex> lock(stateMutex);

         if (bool* buttonPointer = state.getButtonPointer(button))
         {
            *buttonPointer = boolValue;
         }
         else if (float* dialPointer = state.getDialPointer(dial))
         {
            *dialPointer = floatValue;
         }
         else if (float* sliderPointer = state.getSliderPointer(slider))
         {
            *sliderPointer = floatValue;
         }
      }

      {
         std::lock_guard<std::recursive_mutex> lock(callbackMutex);

         if (button != Button::None && buttonCallback)
         {
            buttonCallback(button, boolValue);
         }
         else if (dial != Dial::None && dialCallback)
         {
            dialCallback(dial, floatValue);
         }
         else if (slider != Slider::None && sliderCallback)
         {
            sliderCallback(slider, floatValue);
         }
      }
   }
}
