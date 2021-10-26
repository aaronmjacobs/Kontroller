#include "Communicator.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

namespace Kontroller
{
   namespace
   {
      void readThreadRun(Device::Communicator& communicator, int midiFileDescriptor, int pipeReadDescriptor, std::atomic_bool& shuttingDown)
      {
         while (!shuttingDown.load())
         {
            std::array<pollfd, 2> pollData;
            pollData[0].fd = midiFileDescriptor;
            pollData[1].fd = pipeReadDescriptor;
            pollData[0].events = POLLRDNORM;
            pollData[1].events = POLLRDNORM;

            int pollResult = poll(pollData.data(), pollData.size(), -1);
            if (pollResult > 0 && (pollData[0].revents & POLLRDNORM) != 0 && !shuttingDown.load())
            {
               std::array<uint8_t, 3> data;
               ssize_t bytesRead = read(midiFileDescriptor, data.data(), data.size());
               if (bytesRead == data.size())
               {
                  communicator.onMessageReceived(data[1], data[2]);
               }
            }
         }
      }
   }

   struct Device::Communicator::ImplData
   {
      int midiFileDescriptor = -1;
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
      return implData->midiFileDescriptor != -1;
   }

   bool Device::Communicator::connect()
   {
      if (isConnected())
      {
         return true;
      }

      implData->midiFileDescriptor = open("/dev/midi1", O_NONBLOCK, O_RDONLY);
      if (implData->midiFileDescriptor != -1)
      {
         int pipes[2];
         int pipeCreateResult = pipe(pipes);
         if (pipeCreateResult == 0)
         {
            implData->pipeReadDescriptor = pipes[0];
            implData->pipeWriteDescriptor = pipes[1];
         }
      }

      if (implData->pipeReadDescriptor == -1 || implData->pipeWriteDescriptor == -1)
      {
         disconnect();
      }
      else
      {
         implData->readThread = std::thread([this]() { readThreadRun(*this, implData->midiFileDescriptor, implData->pipeReadDescriptor, implData->shuttingDown); });
      }

      return false;
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
      if (implData->midiFileDescriptor != -1)
      {
         close(implData->midiFileDescriptor);
         implData->midiFileDescriptor = -1;
      }
   }

   void Device::Communicator::poll()
   {
   }

   bool Device::Communicator::initializeMessage()
   {
      return true;
   }

   bool Device::Communicator::appendToMessage(uint8_t* data, size_t numBytes)
   {
      return true; // TODO implement
   }

   bool Device::Communicator::finalizeMessage()
   {
      return true;
   }
}
