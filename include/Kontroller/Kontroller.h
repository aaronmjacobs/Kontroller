#ifndef KONTROLLER_H
#define KONTROLLER_H

#include <array>
#include <cstdint>
#include <memory>

enum class KontrolLed {
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

struct KontrolColumn {
   float dial;
   float slider;
   
   bool s;
   bool m;
   bool r;
};

struct KontrolState {
   std::array<KontrolColumn, 8> columns;

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

struct KontrolImplData;

class Kontroller {
private:
   KontrolState state { 0 };
   std::unique_ptr<KontrolImplData> data;

public:
   Kontroller();

   ~Kontroller();

   KontrolState getState() const;

   void enableLedControl(bool enable);

   void setLedOn(KontrolLed led, bool on);

   // Used internally, should not be called
   void update(uint8_t id, uint8_t value);
};

#endif
