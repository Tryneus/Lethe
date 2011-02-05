#ifndef _COMMREGISTRY_H
#define _COMMREGISTRY_H

#include "ThreadComm.h"
#include "ProcessComm.h"
#include "SocketComm.h"

class CommRegistry : Singleton<CommRegistry>
{
public:

  createConnection(Thread& source, Thread& target);
  createConnection(Thread& source, Process& target);
  createConnection(Thread& source, IpAddress& target);

  destroyConnection(Thread& source, Thread& target);
  destroyConnection(Thread& source, Process& target);
  destroyConnection(Thread& source, IpAddress& target);

  struct Trace
  {
    struct Filter
    {

    };
    std::vector<Filter> m_filter;

    class Action
    {
    public:
      virtual void trigger() = 0;
    };

    class PipeAction
    {
    public:
      PipeAction(Pipe& pipe);
      void trigger();
    private:
      Pipe& m_pipe;
    };

    class LogAction
    {
    public:
      LogAction(Log::Level level);
      void trigger();
    };

    class ForwardAction
    {
    public:
      ForwardAction(Connection& conn);
      void trigger();
    private:
      Connection& m_conn;
    };
    std::vector<Action> m_action;
  }

  addTrace(int connection, filter, actions);

private:
  friend class Singleton<CommRegistry>;

  CommRegistry();
  ~CommRegistry();

};

#endif
