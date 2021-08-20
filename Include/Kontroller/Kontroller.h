#pragma once

#include <readerwriterqueue.h>

#include <array>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class Kontroller
{
public:
   enum class Button : uint8_t
   {
      None,
      TrackPrevious,
      TrackNext,
      Cycle,
      MarkerSet,
      MarkerPrevious,
      MarkerNext,
      Rewind,
      FastForward,
      Stop,
      Play,
      Record,
      Group1Solo,
      Group1Mute,
      Group1Record,
      Group2Solo,
      Group2Mute,
      Group2Record,
      Group3Solo,
      Group3Mute,
      Group3Record,
      Group4Solo,
      Group4Mute,
      Group4Record,
      Group5Solo,
      Group5Mute,
      Group5Record,
      Group6Solo,
      Group6Mute,
      Group6Record,
      Group7Solo,
      Group7Mute,
      Group7Record,
      Group8Solo,
      Group8Mute,
      Group8Record
   };

   enum class Dial : uint8_t
   {
      None,
      Group1,
      Group2,
      Group3,
      Group4,
      Group5,
      Group6,
      Group7,
      Group8
   };

   enum class Slider : uint8_t
   {
      None,
      Group1,
      Group2,
      Group3,
      Group4,
      Group5,
      Group6,
      Group7,
      Group8
   };

   enum class LED : uint8_t
   {
      None,
      Cycle,
      Rewind,
      FastForward,
      Stop,
      Play,
      Record,
      Group1Solo,
      Group1Mute,
      Group1Record,
      Group2Solo,
      Group2Mute,
      Group2Record,
      Group3Solo,
      Group3Mute,
      Group3Record,
      Group4Solo,
      Group4Mute,
      Group4Record,
      Group5Solo,
      Group5Mute,
      Group5Record,
      Group6Solo,
      Group6Mute,
      Group6Record,
      Group7Solo,
      Group7Mute,
      Group7Record,
      Group8Solo,
      Group8Mute,
      Group8Record
   };

   struct Group
   {
      float dial = 0.0f;
      float slider = 1.0f;

      bool solo = false;
      bool mute = false;
      bool record = false;
   };

   struct State
   {
      std::array<Group, 8> groups;

      bool trackPrevious = false;
      bool trackNext = false;

      bool cycle = false;

      bool markerSet = false;
      bool markerPrevious = false;
      bool markerNext = false;

      bool rewind = false;
      bool fastForward = false;
      bool stop = false;
      bool play = false;
      bool record = false;
   };

   class Communicator;

   using ButtonCallback = std::function<void(Button, bool)>;
   using DialCallback = std::function<void(Dial, float)>;
   using SliderCallback = std::function<void(Slider, float)>;

   Kontroller();
   ~Kontroller();

   bool isConnected() const
   {
      return communicatorConnected.load();
   }

   State getState() const;

   void enableLEDControl(bool enable);
   void setLEDOn(LED led, bool on);

   void setButtonCallback(ButtonCallback callback);
   void clearButtonCallback();

   void setDialCallback(DialCallback callback);
   void clearDialCallback();

   void setSliderCallback(SliderCallback callback);
   void clearSliderCallback();

   static State getOnlyNewButtons(const State& previous, const State& current);

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
   std::atomic_bool eventPending;
   std::atomic_bool shuttingDown;

   std::atomic_bool communicatorConnected;

   moodycamel::ReaderWriterQueue<MidiMessage> messageQueue;
   moodycamel::ReaderWriterQueue<MidiCommand> commandQueue;

   std::recursive_mutex callbackMutex;
   ButtonCallback buttonCallback;
   DialCallback dialCallback;
   SliderCallback sliderCallback;
};
