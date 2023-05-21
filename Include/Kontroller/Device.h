#pragma once

#include "Kontroller/State.h"

#include <readerwriterqueue.h>

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace Kontroller
{
   class Device
   {
   public:
      Device();
      ~Device();

      bool isConnected() const
      {
         return communicatorConnected.load();
      }

      State getState() const;
      void setState(const State& newState);

      void enableLEDControl(bool enable);
      void setLEDOn(LED led, bool on);

      using ButtonCallback = std::function<void(Button, bool)>;
      using DialCallback = std::function<void(Dial, float)>;
      using SliderCallback = std::function<void(Slider, float)>;

      void setButtonCallback(ButtonCallback callback);
      void clearButtonCallback();

      void setDialCallback(DialCallback callback);
      void clearDialCallback();

      void setSliderCallback(SliderCallback callback);
      void clearSliderCallback();

      class Communicator;

   private:
      struct MidiMessage
      {
         uint8_t id = 0;
         uint8_t value = 0;
      };

      struct MidiCommand
      {
         enum class Type : uint8_t
         {
            Control,
            LED
         };

         Type type = Type::Control;
         Kontroller::LED led = Kontroller::LED::None;
         bool value = false;
      };

      void queueMessage(uint8_t id, uint8_t value);

      void threadRun();
      void processMessage(MidiMessage message);

      static const char* const kDeviceName;

      State state;
      mutable std::mutex stateMutex;

      std::thread thread;
      std::condition_variable cv;
      std::mutex eventMutex;
      std::atomic_bool eventPending = { false };
      std::atomic_bool shuttingDown = { false };

      std::atomic_bool communicatorConnected = { false };

      moodycamel::ReaderWriterQueue<MidiMessage> messageQueue;
      moodycamel::ReaderWriterQueue<MidiCommand> commandQueue;

      std::recursive_mutex callbackMutex;
      ButtonCallback buttonCallback;
      DialCallback dialCallback;
      SliderCallback sliderCallback;
   };
}
