#pragma once

#include <array>
#include <cstdint>

namespace Kontroller
{
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

   const char* getName(Button button);
   const char* getName(Dial dial);
   const char* getName(Slider slider);
   const char* getName(LED led);

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

      static State getOnlyNewButtons(const State& previous, const State& current);

      bool* getButtonPointer(Button button);
      float* getDialPointer(Dial dial);
      float* getSliderPointer(Slider slider);
   };
}
