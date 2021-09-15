#include "Kontroller/State.h"

namespace Kontroller
{
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
