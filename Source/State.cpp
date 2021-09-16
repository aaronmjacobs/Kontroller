#include "Kontroller/State.h"

namespace Kontroller
{
   const char* getName(Button button)
   {
      switch (button)
      {
      case Button::None: return "None";
      case Button::TrackPrevious: return "Track Previous";
      case Button::TrackNext: return "Track Next";
      case Button::Cycle: return "Cycle";
      case Button::MarkerSet: return "Marker Set";
      case Button::MarkerPrevious: return "Marker Previous";
      case Button::MarkerNext: return "Marker Next";
      case Button::Rewind: return "Rewind";
      case Button::FastForward: return "Fast Forward";
      case Button::Stop: return "Stop";
      case Button::Play: return "Play";
      case Button::Record: return "Record";
      case Button::Group1Solo: return "Group 1 Solo";
      case Button::Group1Mute: return "Group 1 Mute";
      case Button::Group1Record: return "Group 1 Record";
      case Button::Group2Solo: return "Group 2 Solo";
      case Button::Group2Mute: return "Group 2 Mute";
      case Button::Group2Record: return "Group 2 Record";
      case Button::Group3Solo: return "Group 3 Solo";
      case Button::Group3Mute: return "Group 3 Mute";
      case Button::Group3Record: return "Group 3 Record";
      case Button::Group4Solo: return "Group 4 Solo";
      case Button::Group4Mute: return "Group 4 Mute";
      case Button::Group4Record: return "Group 4 Record";
      case Button::Group5Solo: return "Group 5 Solo";
      case Button::Group5Mute: return "Group 5 Mute";
      case Button::Group5Record: return "Group 5 Record";
      case Button::Group6Solo: return "Group 6 Solo";
      case Button::Group6Mute: return "Group 6 Mute";
      case Button::Group6Record: return "Group 6 Record";
      case Button::Group7Solo: return "Group 7 Solo";
      case Button::Group7Mute: return "Group 7 Mute";
      case Button::Group7Record: return "Group 7 Record";
      case Button::Group8Solo: return "Group 8 Solo";
      case Button::Group8Mute: return "Group 8 Mute";
      case Button::Group8Record: return "Group 8 Record";
      default: return nullptr;
      }
   }

   const char* getName(Dial dial)
   {
      switch (dial)
      {
      case Dial::None: return "None";
      case Dial::Group1: return "Group 1";
      case Dial::Group2: return "Group 2";
      case Dial::Group3: return "Group 3";
      case Dial::Group4: return "Group 4";
      case Dial::Group5: return "Group 5";
      case Dial::Group6: return "Group 6";
      case Dial::Group7: return "Group 7";
      case Dial::Group8: return "Group 8";
      default: return nullptr;
      }
   }

   const char* getName(Slider slider)
   {
      switch (slider)
      {
      case Slider::None: return "None";
      case Slider::Group1: return "Group 1";
      case Slider::Group2: return "Group 2";
      case Slider::Group3: return "Group 3";
      case Slider::Group4: return "Group 4";
      case Slider::Group5: return "Group 5";
      case Slider::Group6: return "Group 6";
      case Slider::Group7: return "Group 7";
      case Slider::Group8: return "Group 8";
      default: return nullptr;
      }
   }

   const char* getName(LED led)
   {
      switch (led)
      {
      case LED::None: return "None";
      case LED::Cycle: return "Cycle";
      case LED::Rewind: return "Rewind";
      case LED::FastForward: return "Fast Forward";
      case LED::Stop: return "Stop";
      case LED::Play: return "Play";
      case LED::Record: return "Record";
      case LED::Group1Solo: return "Group 1 Solo";
      case LED::Group1Mute: return "Group 1 Mute";
      case LED::Group1Record: return "Group 1 Record";
      case LED::Group2Solo: return "Group 2 Solo";
      case LED::Group2Mute: return "Group 2 Mute";
      case LED::Group2Record: return "Group 2 Record";
      case LED::Group3Solo: return "Group 3 Solo";
      case LED::Group3Mute: return "Group 3 Mute";
      case LED::Group3Record: return "Group 3 Record";
      case LED::Group4Solo: return "Group 4 Solo";
      case LED::Group4Mute: return "Group 4 Mute";
      case LED::Group4Record: return "Group 4 Record";
      case LED::Group5Solo: return "Group 5 Solo";
      case LED::Group5Mute: return "Group 5 Mute";
      case LED::Group5Record: return "Group 5 Record";
      case LED::Group6Solo: return "Group 6 Solo";
      case LED::Group6Mute: return "Group 6 Mute";
      case LED::Group6Record: return "Group 6 Record";
      case LED::Group7Solo: return "Group 7 Solo";
      case LED::Group7Mute: return "Group 7 Mute";
      case LED::Group7Record: return "Group 7 Record";
      case LED::Group8Solo: return "Group 8 Solo";
      case LED::Group8Mute: return "Group 8 Mute";
      case LED::Group8Record: return "Group 8 Record";
      default: return nullptr;
      }
   }

   // static
   State State::getOnlyNewButtons(const State& previous, const State& current)
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

   bool* State::getButtonPointer(Button button)
   {
      switch (button)
      {
      case Button::TrackPrevious: return &trackPrevious;
      case Button::TrackNext: return &trackNext;
      case Button::Cycle: return &cycle;
      case Button::MarkerSet: return &markerSet;
      case Button::MarkerPrevious: return &markerPrevious;
      case Button::MarkerNext: return &markerNext;
      case Button::Rewind: return &rewind;
      case Button::FastForward: return &fastForward;
      case Button::Stop: return &stop;
      case Button::Play: return &play;
      case Button::Record: return &record;
      case Button::Group1Solo: return &groups[0].solo;
      case Button::Group1Mute: return &groups[0].mute;
      case Button::Group1Record: return &groups[0].record;
      case Button::Group2Solo: return &groups[1].solo;
      case Button::Group2Mute: return &groups[1].mute;
      case Button::Group2Record: return &groups[1].record;
      case Button::Group3Solo: return &groups[2].solo;
      case Button::Group3Mute: return &groups[2].mute;
      case Button::Group3Record: return &groups[2].record;
      case Button::Group4Solo: return &groups[3].solo;
      case Button::Group4Mute: return &groups[3].mute;
      case Button::Group4Record: return &groups[3].record;
      case Button::Group5Solo: return &groups[4].solo;
      case Button::Group5Mute: return &groups[4].mute;
      case Button::Group5Record: return &groups[4].record;
      case Button::Group6Solo: return &groups[5].solo;
      case Button::Group6Mute: return &groups[5].mute;
      case Button::Group6Record: return &groups[5].record;
      case Button::Group7Solo: return &groups[6].solo;
      case Button::Group7Mute: return &groups[6].mute;
      case Button::Group7Record: return &groups[6].record;
      case Button::Group8Solo: return &groups[7].solo;
      case Button::Group8Mute: return &groups[7].mute;
      case Button::Group8Record: return &groups[7].record;
      default: return nullptr;
      }
   }

   float* State::getDialPointer(Dial dial)
   {
      switch (dial)
      {
      case Dial::Group1: return &groups[0].dial;
      case Dial::Group2: return &groups[1].dial;
      case Dial::Group3: return &groups[2].dial;
      case Dial::Group4: return &groups[3].dial;
      case Dial::Group5: return &groups[4].dial;
      case Dial::Group6: return &groups[5].dial;
      case Dial::Group7: return &groups[6].dial;
      case Dial::Group8: return &groups[7].dial;
      default: return nullptr;
      }
   }

   float* State::getSliderPointer(Slider slider)
   {
      switch (slider)
      {
      case Slider::Group1: return &groups[0].slider;
      case Slider::Group2: return &groups[1].slider;
      case Slider::Group3: return &groups[2].slider;
      case Slider::Group4: return &groups[3].slider;
      case Slider::Group5: return &groups[4].slider;
      case Slider::Group6: return &groups[5].slider;
      case Slider::Group7: return &groups[6].slider;
      case Slider::Group8: return &groups[7].slider;
      default: return nullptr;
      }
   }
}
