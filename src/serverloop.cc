#include "serverloop.hh"
#include "tcplistener.hh"
#include "taskpool.hh"
#include <csignal>

void ServerLoop::setHandler(Http::Method method, std::string uri, RequestHandler handler)
{
  method2uri2handler_[method][std::move(uri)] = std::move(handler);
}

void ServerLoop::exec(int port, int numThreads)
{
  auto listener = TcpListener(port);
  auto taskpool = TaskPool(numThreads);

  for(;;)
  {
    taskpool.push([this, conn = listener.acceptConnShared()]
    {
      Http::Request req;

      try
      {
        req.receive(conn.fd());

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
  auto uri2handlerIt = method2uri2handler_.find(req.method());

  if(uri2handlerIt == method2uri2handler_.end())
    return {};

  auto &uri2handler = uri2handlerIt->second;

  auto handlerIt = uri2handler.lower_bound(req.uri());

  if(handlerIt != uri2handler.end() && req.uri() == handlerIt->first)
    return {handlerIt->second(req)};

  if(handlerIt != uri2handler.begin() && req.uri().starts_with((--handlerIt)->first))
    return {handlerIt->second(req)};

  return {};
}

void ServerLoop::initSignalHandling()
{
  signal(SIGPIPE, SIG_IGN);
}
