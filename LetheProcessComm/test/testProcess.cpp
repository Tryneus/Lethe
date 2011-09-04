#include "Lethe.h"
#include "ProcessComm.h"
#include <iostream>

enum ConnectionType
{
  NoType,
  TempConnection,
  HandleConnection,
  ByteConnection,
  MessageConnection
};

enum ConnectionAction
{
  NoAction,
  Echo,
  Send,
  Exit
};

const uint32_t defaultTimeout = 2000;

void doTempStream(uint32_t parentId, ConnectionAction action, uint32_t count);
void doHandleTransfer(uint32_t parentId, ConnectionAction action, uint32_t count);
void doByteStream(uint32_t parentId, ConnectionAction action, uint32_t count);
void doMessageStream(uint32_t parentId, ConnectionAction action, uint32_t count);

int main(int argc, char* argv[])
{
  // Based on the parameters, create a connection to the parent process
  ConnectionType type(NoType);
  ConnectionAction action(NoAction);
  uint32_t sendLimit = 1;
  uint32_t parentId = lethe::getParentProcessId();

  std::cout << "Test Process started with " << argc << " arguments:";

  for(uint32_t i = 0; i < argc; ++i)
  {
    std::cout << " " << argv[i];
  }

  std::cout << std::endl;

  for(uint32_t i = 1; i < argc; ++i)
  {
    std::string option = argv[i];

    if(option == "--echo" && action == NoAction)
      action = Echo;
    else if(option == "--exit" && action == NoAction)
      action = Exit;
    else if(option == "--send" && action == NoAction)
    {
      if(i + 1 > argc)
      {
        std::cout << "No send count specified" << std::endl;
        exit(-1);
      }

      action = Send;
      sendLimit = atoi(argv[i]);

      if(sendLimit == 0)
      {
        std::cout << "invalid send count" << std::endl;
        exit(-1);
      }
    }
    else if(option == "--temp-stream" && type == NoType)
      type = TempConnection;
    else if(option == "--handle-transfer" && type == NoType)
      type = HandleConnection;
    else if(option == "--byte-stream" && type == NoType)
      type = ByteConnection;
    else if(option == "--message-stream" && type == NoType)
      type = MessageConnection;
  }

  if(action == NoAction || type == NoType)
  {
    std::cout << "both an action and a type must be specified" << std::endl;
    exit(-1);
  }


  try
  {
    switch(type)
    {
    case TempConnection:
      doTempStream(parentId, action, sendLimit);
      std::cout << "test process done with TempStream" << std::endl;
      break;
    case HandleConnection:
      doHandleTransfer(parentId, action, sendLimit);
      std::cout << "test process done with HandleTransfer" << std::endl;
      break;
    case ByteConnection:
      doByteStream(parentId, action, sendLimit);
      std::cout << "test process done with ByteStream" << std::endl;
      break;
    case MessageConnection:
      doMessageStream(parentId, action, sendLimit);
      std::cout << "test process done with MessageStream" << std::endl;
      break;
    default:
      std::cout << "invalid action specified" << std::endl;
      break;
    }
  }
  catch(std::exception& ex)
  {
    std::cout << "test process failed with exception: " << ex.what() << std::endl;
  }
}

void doTempStream(uint32_t parentId, ConnectionAction action, uint32_t count)
{
  lethe::TempProcessStream stream(parentId);

  switch(action)
  {
  case Echo:
    {
      char buffer[100];
      uint32_t size;

      while(lethe::WaitForObject(stream, defaultTimeout) == lethe::WaitSuccess)
      {
        size = stream.receive(buffer, sizeof(buffer));
        stream.send(buffer, size);
      }
    }
    break;
  case Send:
    {
      for(; count > 0; --count)
        stream.send(reinterpret_cast<char*>(&count), 1);
    }
    break;
  case Exit:
    break;
  default:
    std::cout << "invalid action specified" << std::endl;
    break;
  }
}

void doHandleTransfer(uint32_t parentId, ConnectionAction action, uint32_t count)
{
  lethe::HandleTransfer transfer(parentId, defaultTimeout);
  std::vector<lethe::Semaphore*> semaphores;

  switch(action)
  {
  case Echo:
    {
      while(true)
      {
        lethe::Semaphore* newSem = transfer.recvSemaphore(defaultTimeout);
        semaphores.push_back(newSem);
        transfer.sendSemaphore(*newSem);
      }
    }
    break;
  case Send:
    {
      for(; count > 0; --count)
      {
        lethe::Semaphore* newSem = new lethe::Semaphore(10, 0);
        semaphores.push_back(newSem);
        transfer.sendSemaphore(*newSem);
      }
    }
    break;
  case Exit:
    break;
  default:
    std::cout << "invalid action specified" << std::endl;
    break;
  }

  for(uint32_t i = 0; i < semaphores.size(); ++i)
    delete semaphores[i];
}

void doByteStream(uint32_t parentId, ConnectionAction action, uint32_t count)
{
  lethe::ProcessByteStream stream(parentId, defaultTimeout);

  switch(action)
  {
  case Echo:
    {
      char buffer[100];
      uint32_t size;

      while(lethe::WaitForObject(stream, defaultTimeout) == lethe::WaitSuccess)
      {
        size = stream.receive(buffer, sizeof(buffer));
        stream.send(buffer, size);
      }
    }
    break;
  case Send:
    {
      for(; count > 0; --count)
        stream.send(reinterpret_cast<char*>(&count), 1);
    }
    break;
  case Exit:
    break;
  default:
    std::cout << "invalid action specified" << std::endl;
    break;
  }
}

void doMessageStream(uint32_t parentId, ConnectionAction action, uint32_t count)
{
  lethe::ProcessMessageStream stream(parentId, 65536, defaultTimeout);

  switch(action)
  {
  case Echo:
    {
      while(lethe::WaitForObject(stream, defaultTimeout) == lethe::WaitSuccess)
      {
        void* buffer = stream.receive();
        uint32_t size = stream.size(buffer);

        void* response = stream.allocate(size);
        memcpy(response, buffer, size);

        stream.release(buffer);
        stream.send(response);
      }
    }
    break;
  case Send:
    {
      for(; count > 0; --count)
      {
        uint32_t* buffer = reinterpret_cast<uint32_t*>(stream.allocate(100));
        stream.send(buffer);
      }
    }
    break;
  case Exit:
    break;
  default:
    std::cout << "invalid action specified" << std::endl;
    break;
  }
}
