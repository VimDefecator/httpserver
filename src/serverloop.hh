#ifndef SERVERLOOP_HH
#define SERVERLOOP_HH

#include "tcplistener.hh"
#include "http.hh"
#include <functional>

class ServerLoop
{
public:
  using RequestHandler = std::function<std::optional<Http::Response>(const Http::Request &)>;

  ServerLoop(int port);

  void addHandler(RequestHandler handler);
  void exec();

private:
  std::optional<Http::Response> handleRequest(const Http::Request &req);

private:
  TcpListener listener_;
  std::vector<RequestHandler> handlers_;
};

#endif
