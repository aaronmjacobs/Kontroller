#pragma once

#include <cstdint>

namespace Kontroller
{
   const char* const kPort = "40807";

   struct EventPacket
   {
      enum Type : uint16_t
      {
         Button = 0x0001,
         Dial = 0x0002,
         Slider = 0x0003
      };

      uint16_t type = 0;
      uint16_t id = 0;
      uint32_t value = 0;
   };
}
