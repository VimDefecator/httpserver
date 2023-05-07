#pragma once

#include <memory>
#include <string>

class AcceptedConnImpl;

class AcceptedConn
{
public:
  AcceptedConn(int sock);

  ~AcceptedConn();

  int fd() const;

private:
  std::unique_ptr<AcceptedConnImpl> impl_;
};

class AcceptedConnShared
{
public:
  AcceptedConnShared(int sock);

  ~AcceptedConnShared();

  int fd() const;

private:
  std::shared_ptr<AcceptedConnImpl> impl_;
};

class TcpListener
{
public:
  TcpListener(int port);
  ~TcpListener();

  AcceptedConn acceptConn() const;
  AcceptedConnShared acceptConnShared() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

