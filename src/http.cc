#include "http.hh"
#include "utils.hh"
#include <algorithm>
#include <cctype>
#include <unistd.h>

using namespace Http;

namespace
{
  std::string_view methodEnum2str(Method method)
  {
    switch(method)
    {
      case Method::Options: return "OPTIONS";
      case Method::Get:     return "GET";
      case Method::Head:    return "HEAD";
      case Method::Post:    return "POST";
      case Method::Put:     return "PUT";
      case Method::Delete:  return "DELETE";
      case Method::Trace:   return "TRACE";
      case Method::Connect: return "CONNECT";
    }
  }

  Method methodStr2enum(std::string_view str)
  {
    static const Method allMethods[] =
    {
      Method::Options,
      Method::Get,
      Method::Head,
      Method::Post,
      Method::Put,
      Method::Delete,
      Method::Trace,
      Method::Connect
    };

    for(auto m : allMethods)
      if(methodEnum2str(m) == str)
        return m;

    throw std::runtime_error("Unrecognized method: " + std::string(str));
  }

  template<class Fn1, class Fn2>
  void processHeadBuf(std::vector<char> &buf, Fn1 &&onFirstLine, Fn2 &&onHeaderLine)
  {
    auto head = std::string_view(buf.data(), buf.size());

    size_t begLinePos, endLinePos;

    begLinePos = 0;
    endLinePos = head.find("\r\n");

    onFirstLine(begLinePos, endLinePos);

    while(true)
    {
      begLinePos = endLinePos + 2;
      endLinePos = head.find("\r\n", begLinePos);

      if(endLinePos == std::string_view::npos || endLinePos == begLinePos)
        break;

      onHeaderLine(begLinePos, endLinePos);
    }
  }

  std::pair<Method, std::string_view> processRequestLine(std::vector<char> &buf, size_t from, size_t to)
  {
    auto line = std::string_view(&buf[from], to - from);

    auto endMethodPos = line.find(' ');
    auto begUriPos = endMethodPos + 1;
    auto endUriPos = line.find(' ', begUriPos);

    return {methodStr2enum(line.substr(0, endMethodPos)),
            line.substr(begUriPos, endUriPos - begUriPos)};
  }

  int processResponseLine(std::vector<char> &buf, size_t from, size_t to)
  {
    auto line = std::string_view(&buf[from], to - from);

    auto endHttpPos = line.find(' ');
    auto begCodePos = endHttpPos + 1;
    auto endCodePos = line.find(' ', begCodePos);

    return str2num<int>(line.substr(begCodePos, endCodePos - begCodePos));
  }
  
  std::pair<std::string_view, std::string_view> processHeaderLine(std::vector<char> &buf, size_t from, size_t to)
  {
    auto line = std::string_view(&buf[from], to - from);
  
    auto endNamePos = line.find_first_of(" \t\r\n:");
    auto colPos = line.find(':', endNamePos);
    auto valPos = line.find_first_not_of(" \t\r\n", colPos + 1);

    for(auto i = from; i < from + endNamePos; ++i)
      buf[i] = tolower(buf[i]);

    return {line.substr(0, endNamePos),
            line.substr(valPos)};
  }

  std::string_view migrateStrView(std::string_view oldStrView,
                                  const std::vector<char> &oldBuf,
                                  const std::vector<char> &newBuf)
  {
    return std::string_view(&newBuf[std::distance(oldBuf.data(), oldStrView.data())], oldStrView.size());
  }

  std::vector<char> &operator+=(std::vector<char> &vec, std::string_view str)
  {
    vec.insert(vec.end(), str.begin(), str.end());
    return vec;
  }
}

void PayloadBase::copyFrom(const PayloadBase &r)
{
  headBuf_ = r.headBuf_;
  bodyBuf_ = r.bodyBuf_;
  bodyStr_ = r.bodyStr_;
  isBodyStr_ = r.isBodyStr_;
  for(auto [hName, hValue] : r.headers_)
    headers_.emplace_back(migrateStrView(hName, r.headBuf_, headBuf_), migrateStrView(hValue, r.headBuf_, headBuf_));
}

std::optional<std::string_view> PayloadBase::findHeader(std::string_view name) const
{
  for(auto [hName, value] : headers_)
    if(hName == name)
      return {value};

  return {};
}

void PayloadBase::addHeader(std::string_view name, std::string_view value)
{
  headBuf_ += name;
  headBuf_ += ": ";
  headBuf_ += value;
  headBuf_ += "\r\n";
}

void PayloadBase::setBody(std::vector<char> body)
{
  addHeader("content-length", std::to_string(body.size()));
  bodyBuf_ = std::move(body);
  bodyStr_.clear();
  isBodyStr_ = false;
}

void PayloadBase::setBody(std::string body)
{
  addHeader("content-length", std::to_string(body.size()));
  bodyBuf_.clear();
  bodyStr_ = std::move(body);
  isBodyStr_ = true;
}

void PayloadBase::send(int fd)
{
  if(isBodyStr_)
    send(fd, bodyStr_.data(), bodyStr_.size());
  else
    send(fd, bodyBuf_.data(), bodyBuf_.size());
}

void PayloadBase::send(int fd, std::string_view body)
{
  send(fd, body.data(), body.size());
}

void PayloadBase::send(int fd, const void *bodyData, size_t bodySize)
{
  write(fd, headBuf_.data(), headBuf_.size());
  write(fd, "\r\n", 2);
  write(fd, bodyData, bodySize);
}

void PayloadBase::readHeadBuf(int fd)
{
  static const std::string_view rnrn = "\r\n\r\n";

  size_t pos = 0, endHeadersPos;

  headBuf_.resize(0x100);

  while(true)
  {
    if(pos == headBuf_.size())
      headBuf_.resize(headBuf_.size() * 2);

    auto nread = throwOnErr(read(fd, &headBuf_[pos], headBuf_.size() - pos));

    auto rnrnIt = std::search(pos < 3 ? headBuf_.begin() : headBuf_.begin() + pos - 3,
                              headBuf_.begin() + pos + nread,
                              rnrn.begin(),
                              rnrn.end());
    pos += nread;

    if(rnrnIt != headBuf_.end())
    {
      endHeadersPos = std::distance(headBuf_.begin(), rnrnIt) + 4;
      break;
    }
  }

  std::copy(headBuf_.begin() + endHeadersPos,
            headBuf_.begin() + pos,
            std::back_inserter(bodyBuf_));

  headBuf_.resize(endHeadersPos);
}

void PayloadBase::readBodyBuf(int fd)
{
  if(auto contentLength = findHeader("content-length"))
  {
    size_t pos, len;
    pos = bodyBuf_.size();
    len = str2num<size_t>(*contentLength);

    bodyBuf_.resize(len);

    while(pos < len)
      pos += throwOnErr(read(fd, &bodyBuf_[pos], len - pos));
  }
}

void Request::copyFrom(const Request &r)
{
  method_ = r.method_;
  uri_ = migrateStrView(r.uri_, r.headBuf_, headBuf_);
}

void Request::setMethodUri(Method method, std::string_view uri)
{
  auto methodStr = methodEnum2str(method);

  headBuf_ += methodStr;
  headBuf_ += " ";
  headBuf_ += uri;
  headBuf_ += " HTTP/1.1\r\n";

  uri_ = dumpHead().substr(methodStr.size() + 1, uri.size());
}

void Request::receive(int fd)
{
  readHeadBuf(fd);

  processHeadBuf(headBuf_,
    [&](size_t from, size_t to){
      std::tie(method_, uri_) = processRequestLine(headBuf_, from, to);
    },
    [&](size_t from, size_t to){
      headers_.push_back(processHeaderLine(headBuf_, from, to));
    });

  readBodyBuf(fd);
}

namespace
{
  std::string_view statusCode2reasonPhrase(int code)
  {
    switch(code)
    {
      case 100: return "Continue";
      case 101: return "Switching Protocols";
      case 200: return "OK";
      case 201: return "Created";
      case 202: return "Accepted";
      case 203: return "Non-Authoritative Information";
      case 204: return "No Content";
      case 205: return "Reset Content";
      case 206: return "Partial Content";
      case 300: return "Multiple Choices";
      case 301: return "Moved Permanently";
      case 302: return "Found";
      case 303: return "See Other";
      case 304: return "Not Modified";
      case 305: return "Use Proxy";
      case 307: return "Temporary Redirect";
      case 400: return "Bad Request";
      case 401: return "Unauthorized";
      case 402: return "Payment Required";
      case 403: return "Forbidden";
      case 404: return "Not Found";
      case 405: return "Method Not Allowed";
      case 406: return "Not Acceptable";
      case 407: return "Proxy Authentication Required";
      case 408: return "Request Time-out";
      case 409: return "Conflict";
      case 410: return "Gone";
      case 411: return "Length Required";
      case 412: return "Precondition Failed";
      case 413: return "Request Entity Too Large";
      case 414: return "Request-URI Too Large";
      case 415: return "Unsupported Media Type";
      case 416: return "Requested range not satisfiable";
      case 417: return "Expectation Failed";
      case 500: return "Internal Server Error";
      case 501: return "Not Implemented";
      case 502: return "Bad Gateway";
      case 503: return "Service Unavailable";
      case 504: return "Gateway Time-out";
      case 505: return "HTTP Version not supported";
      default : return "";
    }
  }
}

void Response::setCode(int code)
{
  code_ = code;

  headBuf_ += "HTTP/1.1 ";
  headBuf_ += std::to_string(code);
  headBuf_ += " ";
  headBuf_ += statusCode2reasonPhrase(code);
  headBuf_ += "\r\n";
}

void Response::receive(int fd)
{
  readHeadBuf(fd);

  processHeadBuf(headBuf_,
    [&](size_t from, size_t to){
      code_ = processResponseLine(headBuf_, from, to);
    },
    [&](size_t from, size_t to){
      headers_.push_back(processHeaderLine(headBuf_, from, to));
    });

  readBodyBuf(fd);
}
