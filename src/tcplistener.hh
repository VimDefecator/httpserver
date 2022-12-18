#ifndef TCPLISTENER_HH
#define TCPLISTENER_HH

#include <memory>
#include <string>

class AcceptedConn
{
public:
  AcceptedConn() = default;
  AcceptedConn(int sock);

  ~AcceptedConn();

  int fd();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

class TcpListener
{
public:
  TcpListener(int port);
  ~TcpListener();

  AcceptedConn acceptConn();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
