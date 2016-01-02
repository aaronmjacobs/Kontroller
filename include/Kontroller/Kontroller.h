#ifndef KONTROLLER_H
#define KONTROLLER_H

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>

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

   struct ImplData;

private:
   State state { 0 };
   mutable std::mutex mutex;
   std::unique_ptr<ImplData> data;

public:
   Kontroller();

   ~Kontroller();

   State getState() const;

   void enableLEDControl(bool enable);

   void setLEDOn(LED led, bool on);

   // Used internally, should not be called
   void update(uint8_t id, uint8_t value);
};

#endif
