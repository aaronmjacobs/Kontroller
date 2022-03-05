#include "Communicator.h"

#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <thread>

namespace Kontroller
{
   namespace
   {
      void readThreadRun(Device::Communicator& communicator, snd_rawmidi_t* midiInput, pollfd midiPollData, int pipeReadDescriptor, std::atomic_bool& shuttingDown)
      {
         while (!shuttingDown.load())
         {
            std::array<pollfd, 2> pollData;
            pollData[0] = midiPollData;
            pollData[1].fd = pipeReadDescriptor;
            pollData[1].events = POLLRDNORM;

            int pollResult = poll(pollData.data(), pollData.size(), -1);
            if (pollResult > 0)
            {
               if ((pollData[0].revents & POLLIN) != 0 && !shuttingDown.load())
               {
                  std::array<uint8_t, 3> data;
                  ssize_t bytesRead = snd_rawmidi_read(midiInput, data.data(), data.size());
                  if (bytesRead == data.size() && data[0] == kControlCommand)
                  {
                     communicator.onMessageReceived(data[1], data[2]);
                  }
               }
               else if ((pollData[0].revents & POLLERR) != 0)
               {
                  communicator.onConnectionLost();
               }
            }
         }
      }
   }

   struct Device::Communicator::ImplData
   {
      snd_rawmidi_t* midiInput = nullptr;
      snd_rawmidi_t* midiOutput = nullptr;
      pollfd pollData = {};
      int pipeReadDescriptor = -1;
      int pipeWriteDescriptor = -1;
      std::thread readThread;
      std::atomic_bool shuttingDown;
   };

   Device::Communicator::Communicator(Device& owningDevice)
      : device(owningDevice)
      , implData(std::make_unique<ImplData>())
   {
   }

   Device::Communicator::~Communicator()
   {
      disconnect();
   }

   bool Device::Communicator::isConnected() const
   {
      return implData->midiInput != nullptr && implData->midiOutput != nullptr;
   }

   bool Device::Communicator::connect()
   {
      if (isConnected())
      {
         return true;
      }

      std::array<char, 32> portName{};
      std::snprintf(portName.data(), portName.size(), "hw:%s,0", Device::kDeviceName);

      int openResult = snd_rawmidi_open(&implData->midiInput, &implData->midiOutput, portName.data(), SND_RAWMIDI_NONBLOCK);
      if (openResult >= 0 && isConnected())
      {
         int numPollDescriptors = snd_rawmidi_poll_descriptors(implData->midiInput, &implData->pollData, 1);
         if (numPollDescriptors == 1)
         {
            int pipes[2];
            int pipeCreateResult = pipe(pipes);
            if (pipeCreateResult == 0)
            {
               implData->pipeReadDescriptor = pipes[0];
               implData->pipeWriteDescriptor = pipes[1];
            }
         }
      }

      if (implData->pipeReadDescriptor == -1 || implData->pipeWriteDescriptor == -1)
      {
         disconnect();
      }
      else
      {
         implData->readThread = std::thread([this]() { readThreadRun(*this, implData->midiInput, implData->pollData, implData->pipeReadDescriptor, implData->shuttingDown); });
      }

      return isConnected();
   }

   void Device::Communicator::disconnect()
   {
      implData->shuttingDown.store(true);

      if (implData->pipeWriteDescriptor != -1)
      {
         write(implData->pipeWriteDescriptor, "wakey wakey", 1);
      }

      if (implData->readThread.joinable())
      {
         implData->readThread.join();
      }

      implData->shuttingDown.store(false);

      if (implData->pipeWriteDescriptor != -1)
      {
         close(implData->pipeWriteDescriptor);
         implData->pipeWriteDescriptor = -1;
      }
      if (implData->pipeReadDescriptor != -1)
      {
         close(implData->pipeReadDescriptor);
         implData->pipeReadDescriptor = -1;
      }

      if (implData->midiInput)
      {
         snd_rawmidi_close(implData->midiInput);
         implData->midiInput = nullptr;
      }
      if (implData->midiOutput)
      {
         snd_rawmidi_close(implData->midiOutput);
         implData->midiOutput = nullptr;
      }

      implData->pollData = {};
   }

   void Device::Communicator::poll()
   {
      checkForLostConnection();
   }

   bool Device::Communicator::initializeMessage()
   {
      return true;
   }

   bool Device::Communicator::appendToMessage(uint8_t* data, size_t numBytes)
   {
      ssize_t bytesWritten = snd_rawmidi_write(implData->midiOutput, data, numBytes);
      return bytesWritten >= 0 && bytesWritten == numBytes;
   }

   bool Device::Communicator::finalizeMessage()
   {
      return true;
   }
}
