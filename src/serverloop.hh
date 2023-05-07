#pragma once

#include "http.hh"
#include <functional>

class ServerLoop
{
public:
  using RequestHandler = std::function<Http::Response(const Http::Request &)>;

  void setHandler(Http::Method method, std::string uri, RequestHandler handler);
  void exec(int port, int numThreads);

  static void initSignalHandling();

private:
  std::optional<Http::Response> handleRequest(const Http::Request &req);

private:
  std::map<Http::Method, std::map<std::string, RequestHandler, std::less<>>> method2uri2handler_;
};

