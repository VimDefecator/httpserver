#include "tcplistener.hh"
#include "utils.hh"
#include <string>
#include <map>
#include <mutex>
#include <stdexcept>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class AcceptedConnImpl
{
public:
  AcceptedConnImpl(int sock)
  {
    zfill(&addr_);
    addrLen_ = sizeof(addr_);

    fd_ = throwOnErr(
      accept(sock, (sockaddr *)&addr_, &addrLen_),
      "accept connection");
  }

  ~AcceptedConnImpl()
  {
    throwOnErr(
      close(fd_),
      "close connection");
  }

  int fd() const { return fd_; }

private:
  int fd_;
  sockaddr_in addr_;
  socklen_t addrLen_;
};

AcceptedConn::AcceptedConn(int sock)
  : impl_(new AcceptedConnImpl(sock))
{
}

AcceptedConn::~AcceptedConn()
{
}

int AcceptedConn::fd() const
{
  return impl_->fd();
}

AcceptedConnShared::AcceptedConnShared(int sock)
  : impl_(new AcceptedConnImpl(sock))
{
}

AcceptedConnShared::~AcceptedConnShared()
{
}

int AcceptedConnShared::fd() const
{
  return impl_->fd();
}

class TcpListener::Impl
{
public:
  Impl(int port)
  {
    zfill(&addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);

    sock_ = throwOnErr(
      socket(AF_INET, SOCK_STREAM, 0),
      "create socket");

    throwOnErr(
      bind(sock_, (sockaddr *)&addr_, sizeof(addr_)),
      "bind sockaddr");

    throwOnErr(
      listen(sock_, 5),
      "start listen");
  }

  AcceptedConn acceptConn() const
  {
    return AcceptedConn(sock_);
  }

  AcceptedConnShared acceptConnShared() const
  {
    return AcceptedConnShared(sock_);
  }

private:
  int sock_;
  sockaddr_in addr_;
};

TcpListener::TcpListener(int port)
  : impl_(new Impl(port))
{
}

TcpListener::~TcpListener()
{
}

AcceptedConn TcpListener::acceptConn() const
{
  return impl_->acceptConn();
}

AcceptedConnShared TcpListener::acceptConnShared() const
{
  return impl_->acceptConnShared();
}
