#ifndef KONTROLLER_H
#define KONTROLLER_H

#include "readerwriterqueue.h"

#include <array>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

class Kontroller {
public:
   enum class LED {
      kCycle,
      kRewind,
      kFastForward,
      kStop,
      kPlay,
      kRecord,
      kCol1S,
      kCol1M,
      kCol1R,
      kCol2S,
      kCol2M,
      kCol2R,
      kCol3S,
      kCol3M,
      kCol3R,
      kCol4S,
      kCol4M,
      kCol4R,
      kCol5S,
      kCol5M,
      kCol5R,
      kCol6S,
      kCol6M,
      kCol6R,
      kCol7S,
      kCol7M,
      kCol7R,
      kCol8S,
      kCol8M,
      kCol8R
   };

   struct Column {
      float dial;
      float slider;

      bool s;
      bool m;
      bool r;
   };

   struct State {
      std::array<Column, 8> columns;

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

   Kontroller();

   ~Kontroller();

   bool isConnected() const;

   State getState() const {
      std::lock_guard<std::mutex> lock(valueMutex);
      return state;
   }

   void enableLEDControl(bool enable);

   void setLEDOn(LED led, bool on);

   static State onlyNewButtons(const State& previous, const State& current) {
      State onlyNew = current;

      for (size_t i = 0; i < onlyNew.columns.size(); ++i) {
         onlyNew.columns[i].s = current.columns[i].s && !previous.columns[i].s;
         onlyNew.columns[i].m = current.columns[i].m && !previous.columns[i].m;
         onlyNew.columns[i].r = current.columns[i].r && !previous.columns[i].r;
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
   std::condition_variable cv;
   std::unique_ptr<Communicator> communicator;
   std::thread thread;

   void update(uint8_t id, uint8_t value);

   void threadRun();

   void processMessage(MidiMessage message);

   void processCommand(MidiCommand command);

   void processControlCommand(bool enable);

   void processLEDCommand(LED led, bool enable);
};

#endif
