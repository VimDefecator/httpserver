#include "serverloop.hh"
#include "tcplistener.hh"
#include "taskpool.hh"
#include <csignal>

void ServerLoop::setHandler(std::string uri, RequestHandler handler)
{
  uri2handler_[std::move(uri)] = std::move(handler);
}

void ServerLoop::exec(int port, int numThreads)
{
  auto listener = TcpListener(port);
  auto taskpool = TaskPool(numThreads);

  for(;;)
  {
    taskpool.push([&, conn = listener.acceptConnShared()]
    {
      Http::Request req;

      try
      {
        req.recieve(conn.fd());

        if(auto res = handleRequest(req))
          res->send(conn.fd());
        else
          Http::Response(404).send(conn.fd());
      }
      catch(...)
      {
        Http::Response(500).send(conn.fd());
      }
    });
  }
}

std::optional<Http::Response> ServerLoop::handleRequest(const Http::Request &req)
{
  auto it = uri2handler_.lower_bound(req.uri());

  if(it != uri2handler_.end() && req.uri() == it->first)
    return {it->second(req)};

  if(it != uri2handler_.begin() && req.uri().starts_with((--it)->first))
    return {it->second(req)};

  return {};
}

void ServerLoop::initSignalHandling()
{
  signal(SIGPIPE, SIG_IGN);
}
