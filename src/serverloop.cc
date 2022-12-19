#include "serverloop.hh"

ServerLoop::ServerLoop(int port)
  : listener_(port)
{
}

void ServerLoop::setHandler(std::string uri, RequestHandler handler)
{
  uri2handler_[std::move(uri)] = handler;
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
  if(auto it = uri2handler_.lower_bound(req.uri()); it != uri2handler_.begin())
    if(const auto &[uri, handler] = *--it; req.uri().starts_with(uri))
      return {handler(req)};

  return {};
}
