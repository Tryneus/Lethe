#include "Abstraction.h"
#include "MessageStream/WindowsProcessMessageStream.h"

WindowsProcessMessageStream::WindowsProcessMessageStream()
{

}

WindowsProcessMessageStream::~WindowsProcessMessageStream()
{

}

WindowsProcessMessageStream::operator WaitObject&()
{

}

Handle WindowsProcessMessageStream::getHandle() const
{

}

void* WindowsProcessMessageStream::allocate(uint32_t size)
{

}

void WindowsProcessMessageStream::send(void* buffer)
{

}

void* WindowsProcessMessageStream::receive()
{

}

void WindowsProcessMessageStream::release(void* buffer)
{

}
