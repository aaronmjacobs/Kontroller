#ifndef KONTROLLER_H
#define KONTROLLER_H

#include <array>
#include <mutex>

#ifdef __APPLE__
#include <CoreMIDI/MIDIServices.h>
#endif // __APPLE__

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

class Kontroller {
private:
   KontrolState state { 0 };
   mutable std::mutex mutex;

#ifdef __APPLE__
   MIDIClientRef client { 0 };
   MIDIPortRef port { 0 };
#endif // __APPLE__

public:
   Kontroller();

   ~Kontroller();

   void update(unsigned char id, unsigned char value);

   KontrolState getState() const;
};

#endif
