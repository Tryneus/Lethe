#ifndef _WINDOWSPIPE_H
#define _WINDOWSPIPE_H

#include "Windows.h"

class WindowsPipe
{
public:
  WindowsPipe();
  ~WindowsPipe();

  void send();
  void receive();

  HANDLE getHandle();

private:
  HANDLE m_pipe;
};

#endif