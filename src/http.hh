#pragma once

#include <string>
#include <vector>
#include <map>
#include "common/classutils.hh"

namespace Http
{
  enum class Method {
    Options, Get, Head, Post, Put, Delete, Trace, Connect
  };

  class PayloadBase
  {
  public:
    PayloadBase() = default;
    PayloadBase(PayloadBase &&) = default;
    PayloadBase(const PayloadBase &r) {
      copyFrom(r);
    }

    PayloadBase &operator=(PayloadBase &&) = default;
    PayloadBase &operator=(const PayloadBase &r) {
      copyFrom(r);
      return *this;
    };

    const std::vector<std::pair<std::string_view, std::string_view>> &headers() const {
      return headers_;
    }
    const std::vector<char> &bodyBuf() const {
      return bodyBuf_;
    }
    const std::string &bodyStr() const {
      return bodyStr_;
    }

    std::string_view dumpHead() const {
      return std::string_view(headBuf_.data(), headBuf_.size());
    }
    std::string_view dumpBody() const {
      if(isBodyStr_)
        return bodyStr_;
      else
        return std::string_view(bodyBuf_.data(), bodyBuf_.size());
    }

    std::optional<std::string_view> findHeader(std::string_view name) const;

    void addHeader(std::string_view name, std::string_view value);

    void setBody(std::string body);
    void setBody(std::vector<char> body);

    void send(int fd);
    void send(int fd, std::string_view body);
    void send(int fd, const void *bodyData, size_t bodySize);

  protected:
    void readHeadBuf(int fd);
    void readBodyBuf(int fd);

  private:
    void copyFrom(const PayloadBase &);

  protected:
    std::vector<char> headBuf_;
    std::vector<char> bodyBuf_;
    std::string bodyStr_;

    std::vector<std::pair<std::string_view, std::string_view>> headers_;
    bool isBodyStr_ = false;
  };

  template<class Type>
  class Payload : public PayloadBase
  {
  public:
    CHAIN_METHOD(Type, wHeader, addHeader);
    CHAIN_METHOD(Type, wBody, setBody);
  };

  class Request : public Payload<Request>
  {
  public:
    Request() = default;
    Request(Request &&) = default;
    Request(Method method, std::string_view uri) {
      setMethodUri(method, uri);
    }
    Request(const Request &r) : Payload(r) {
      copyFrom(r);
    }

    Request &operator=(Request &&) = default;
    Request &operator=(const Request &r) {
      Payload::operator=(r);
      copyFrom(r);
      return *this;
    }

    Method method() const {
      return method_;
    };
    std::string_view uri() const {
      return uri_;
    }

    void setMethodUri(Method method, std::string_view uri);

    CHAIN_METHOD_AUTO(wMethodUri, setMethodUri);

    void receive(int fd);

  private:
    void copyFrom(const Request &);

  private:
    Method method_;
    std::string_view uri_;
  };

  class Response : public Payload<Response>
  {
  public:
    Response() = default;
    Response(int code) {
      setCode(code);
    }

    int code() const {
      return code_;
    }

    void setCode(int code);

    CHAIN_METHOD_AUTO(wCode, setCode);

    void receive(int fd);

  private:
    int code_;  
  };
}

