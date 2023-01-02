#ifndef SERVERLOOP_HH
#define SERVERLOOP_HH

#include "tcplistener.hh"
#include "http.hh"
#include <functional>

class ServerLoop
{
public:
  using RequestHandler = std::function<Http::Response(const Http::Request &)>;

  ServerLoop(int port);

  void setHandler(std::string uri, RequestHandler handler);
  void exec();

  static void initSignalHandling();

private:
  std::optional<Http::Response> handleRequest(const Http::Request &req);

private:
  TcpListener listener_;
  std::map<std::string, RequestHandler, std::less<>> uri2handler_;
};

#endif
