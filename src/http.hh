#ifndef HTTP_HH
#define HTTP_HH

#include <string>
#include <vector>
#include <map>
#include "classutils.hh"

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
      return headRelStrView2strView(uri_);
    }
    const std::vector<std::pair<std::string_view, std::string_view>> &headers() const {
      return headersStrViews_;
    }
    const std::vector<char> &body() const {
      return bodyBuf_;
    }

    std::optional<std::string_view> findHeader(std::string_view name) const;

    void recieve(int fd);

  private:
    void processHeadBuf();
    void processRequestLine(size_t absBegPos, size_t absEndPos);
    void processHeaderLine(size_t absBegPos, size_t absEndPos);
    void generateHeadersStrViews();

  private:
    using RelStrView = std::pair<size_t, size_t>;
    std::string_view headRelStrView2strView(RelStrView rsv) const;

    std::vector<char> headBuf_;
    std::vector<char> bodyBuf_;

    Method method_;
    RelStrView uri_;
    std::vector<std::pair<RelStrView, RelStrView>> headers_;
    std::vector<std::pair<std::string_view, std::string_view>> headersStrViews_;
  };

  class Response
  {
  public:
    Response(int statusCode);

    void addHeader(std::string_view name, std::string_view value);

    void setBody(std::vector<char> body);
    void setBody(std::string_view body);

    CHAINMETHOD(withHeader, addHeader);
    CHAINMETHOD(withBody, setBody);

    void send(int fd);

  private:
    void addContentLengthHeader();

    std::vector<char> headBuf_;
    std::vector<char> bodyBuf_;
  };
}

#endif
