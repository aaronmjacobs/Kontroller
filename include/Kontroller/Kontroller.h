#ifndef KONTROLLER_H
#define KONTROLLER_H

#include "readerwriterqueue.h"

#include <array>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class Kontroller {
public:
   enum class Button {
      kNone,
      kTrackLeft,
      kTrackRight,
      kCycle,
      kMarkerSet,
      kMarkerLeft,
      kMarkerRight,
      kRewind,
      kFastForward,
      kStop,
      kPlay,
      kRecord,
      kGroup1Solo,
      kGroup1Mute,
      kGroup1Record,
      kGroup2Solo,
      kGroup2Mute,
      kGroup2Record,
      kGroup3Solo,
      kGroup3Mute,
      kGroup3Record,
      kGroup4Solo,
      kGroup4Mute,
      kGroup4Record,
      kGroup5Solo,
      kGroup5Mute,
      kGroup5Record,
      kGroup6Solo,
      kGroup6Mute,
      kGroup6Record,
      kGroup7Solo,
      kGroup7Mute,
      kGroup7Record,
      kGroup8Solo,
      kGroup8Mute,
      kGroup8Record
   };

   enum class Dial {
      kNone,
      kGroup1,
      kGroup2,
      kGroup3,
      kGroup4,
      kGroup5,
      kGroup6,
      kGroup7,
      kGroup8
   };

   enum class Slider {
      kNone,
      kGroup1,
      kGroup2,
      kGroup3,
      kGroup4,
      kGroup5,
      kGroup6,
      kGroup7,
      kGroup8
   };

   enum class LED {
      kCycle,
      kRewind,
      kFastForward,
      kStop,
      kPlay,
      kRecord,
      kGroup1Solo,
      kGroup1Mute,
      kGroup1Record,
      kGroup2Solo,
      kGroup2Mute,
      kGroup2Record,
      kGroup3Solo,
      kGroup3Mute,
      kGroup3Record,
      kGroup4Solo,
      kGroup4Mute,
      kGroup4Record,
      kGroup5Solo,
      kGroup5Mute,
      kGroup5Record,
      kGroup6Solo,
      kGroup6Mute,
      kGroup6Record,
      kGroup7Solo,
      kGroup7Mute,
      kGroup7Record,
      kGroup8Solo,
      kGroup8Mute,
      kGroup8Record
   };

   struct Group {
      float dial;
      float slider;

      bool solo;
      bool mute;
      bool record;
   };

   struct State {
      std::array<Group, 8> groups;

      bool trackLeft;
      bool trackRight;

      bool cycle;

      bool markerSet;
      bool markerLeft;
      bool markerRight;

      bool rewind;
      bool fastForward;
      bool stop;
      bool play;
      bool record;
   };

   class Communicator;

   using ButtonCallback = std::function<void(Button, bool)>;
   using DialCallback = std::function<void(Dial, float)>;
   using SliderCallback = std::function<void(Slider, float)>;

   Kontroller();

   ~Kontroller();

   bool isConnected() const;

   State getState() const {
      std::lock_guard<std::mutex> lock(valueMutex);
      return state;
   }

   void enableLEDControl(bool enable);

   void setLEDOn(LED led, bool on);

   void setButtonCallback(ButtonCallback callback) {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      buttonCallback = callback;
   }

   void clearButtonCallback() {
      setButtonCallback({});
   }

   void setDialCallback(DialCallback callback) {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      dialCallback = callback;
   }

   void clearDialCallback() {
      setDialCallback({});
   }

   void setSliderCallback(SliderCallback callback) {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      sliderCallback = callback;
   }

   void clearSliderCallback() {
      setSliderCallback({});
   }

   static State onlyNewButtons(const State& previous, const State& current) {
      State onlyNew = current;

      for (size_t i = 0; i < onlyNew.groups.size(); ++i) {
         onlyNew.groups[i].solo = current.groups[i].solo && !previous.groups[i].solo;
         onlyNew.groups[i].mute = current.groups[i].mute && !previous.groups[i].mute;
         onlyNew.groups[i].record = current.groups[i].record && !previous.groups[i].record;
      }

      onlyNew.trackLeft = current.trackLeft && !previous.trackLeft;
      onlyNew.trackRight = current.trackRight && !previous.trackRight;

      onlyNew.cycle = current.cycle && !previous.cycle;

      onlyNew.markerSet = current.markerSet && !previous.markerSet;
      onlyNew.markerLeft = current.markerLeft && !previous.markerLeft;
      onlyNew.markerRight = current.markerRight && !previous.markerRight;

      onlyNew.rewind = current.rewind && !previous.rewind;
      onlyNew.fastForward = current.fastForward && !previous.fastForward;
      onlyNew.stop = current.stop && !previous.stop;
      onlyNew.play = current.play && !previous.play;
      onlyNew.record = current.record && !previous.record;

      return onlyNew;
   }

private:
   struct MidiMessage {
      uint8_t id;
      uint8_t value;
   };

   struct MidiCommand {
      enum class Type : uint8_t {
         kControl,
         kLED
      };

      Type type;
      Kontroller::LED led;
      bool value;
   };

   static const char* const kDeviceName;

   State state{};
   std::atomic_bool shuttingDown;
   moodycamel::ReaderWriterQueue<MidiMessage> messageQueue;
   moodycamel::ReaderWriterQueue<MidiCommand> commandQueue;
   mutable std::mutex valueMutex;
   std::recursive_mutex callbackMutex;
   std::condition_variable cv;
   std::unique_ptr<Communicator> communicator;
   std::thread thread;
   ButtonCallback buttonCallback;
   DialCallback dialCallback;
   SliderCallback sliderCallback;

   void update(uint8_t id, uint8_t value);

   void threadRun();

   void processMessage(MidiMessage message);

   void processCommand(MidiCommand command);

   void processControlCommand(bool enable);

   void processLEDCommand(LED led, bool enable);
};

#endif
