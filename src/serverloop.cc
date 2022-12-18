#include "serverloop.hh"

ServerLoop::ServerLoop(int port)
  : listener_(port)
{
}

void ServerLoop::addHandler(RequestHandler handler)
{
  handlers_.push_back(std::move(handler));
}

void ServerLoop::exec()
{
  for(;;)
  {
    auto conn = listener_.acceptConn();

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
  }
}

std::optional<Http::Response> ServerLoop::handleRequest(const Http::Request &req)
{
  for(auto &handler : handlers_)
    if(auto res = handler(req))
      return {std::move(res)};

   return {};
}
