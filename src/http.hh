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

  class Request
  {
  public:
    Request() = default;
    Request(Request &&) = default;
    Request(const Request &);

    Request &operator=(Request &&) = default;
    Request &operator=(const Request &);

    Method method() const {
      return method_;
    };
    std::string_view uri() const {
      return uri_;
    }
    const std::vector<std::pair<std::string_view, std::string_view>> &headers() const {
      return headers_;
    }
    const std::vector<char> &body() const {
      return bodyBuf_;
    }
    std::string_view bodyStr() const {
      return std::string_view(bodyBuf_.data(), bodyBuf_.size());
    }

    std::optional<std::string_view> findHeader(std::string_view name) const;

    std::string toString() const {
      return std::string(headBuf_.data(), headBuf_.size()) + std::string(bodyBuf_.data(), bodyBuf_.size());
    }

    void recieve(int fd);

  private:
    void processHeadBuf();

  private:
    std::vector<char> headBuf_;
    std::vector<char> bodyBuf_;

    Method method_;
    std::string_view uri_;
    std::vector<std::pair<std::string_view, std::string_view>> headers_;
  };

  class Response
  {
  public:
    Response(int statusCode);

    void addHeader(std::string_view name, std::string_view value);

    void setBody(std::string body);
    void setBody(std::vector<char> body);

    void emplaceBody(std::string_view body);

    CHAIN_METHOD(wAddHeader, addHeader);
    CHAIN_METHOD(wSetBody, setBody);
    CHAIN_METHOD(wEmplaceBody, emplaceBody);

    void send(int fd);
    void send(int fd, std::string_view body);
    void send(int fd, const void *bodyData, size_t bodySize);

  private:
    std::string headStr_;
    std::string bodyStr_;
    std::vector<char> bodyVec_;

    enum class BodyType { Vec, Str };
    BodyType bodyType_ = BodyType::Vec;
  };
}

