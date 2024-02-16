#include "http.hh"
#include "utils.hh"
#include <algorithm>
#include <cctype>
#include <unistd.h>

using namespace Http;

namespace
{
  Method methodStr2enum(std::string_view str)
  {
    if(str == "OPTIONS") return Method::Options;
    if(str == "GET")     return Method::Get;
    if(str == "HEAD")    return Method::Head;
    if(str == "POST")    return Method::Post;
    if(str == "PUT")     return Method::Put;
    if(str == "DELETE")  return Method::Delete;
    if(str == "TRACE")   return Method::Trace;
    if(str == "CONNECT") return Method::Connect;

    throw std::runtime_error("Unrecognized method: " + std::string(str));
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
}

Request::Request(const Request &r)
{
  *this = r;
}

Request &Request::operator=(const Request &r)
{
  headBuf_ = r.headBuf_;
  bodyBuf_ = r.bodyBuf_;
  method_ = r.method_;
  uri_ = migrateStrView(r.uri_, r.headBuf_, headBuf_);
  for(auto [hName, hValue] : r.headers_)
    headers_.emplace_back(migrateStrView(hName, r.headBuf_, headBuf_), migrateStrView(hValue, r.headBuf_, headBuf_));

  return *this;
}

std::optional<std::string_view> Request::findHeader(std::string_view name) const
{
  for(auto [hName, value] : headers_)
    if(hName == name)
      return {value};

  return {};
}

void Request::recieve(int fd)
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

  processHeadBuf();

  if(auto contentLength = findHeader("content-length"))
  {
    auto len = str2num<size_t>(*contentLength);

    pos -= endHeadersPos;

    bodyBuf_.resize(len);

    while(pos < len)
    {
      pos += throwOnErr(read(fd, &bodyBuf_[pos], len - pos));
    }
  }
}

void Request::processHeadBuf()
{
  auto head = std::string_view(headBuf_.data(), headBuf_.size());

  size_t lineBegPos, lineEndPos;

  lineBegPos = 0;
  lineEndPos = head.find("\r\n");

  std::tie(method_, uri_) = processRequestLine(headBuf_, lineBegPos, lineEndPos);

  while(true)
  {
    lineBegPos = lineEndPos + 2;
    lineEndPos = head.find("\r\n", lineBegPos);

    if(lineEndPos == std::string_view::npos || lineEndPos == lineBegPos)
      break;

    headers_.push_back(processHeaderLine(headBuf_, lineBegPos, lineEndPos));
  }
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

Response::Response(int statusCode)
{
  headStr_ += "HTTP/1.1 ";
  headStr_ += std::to_string(statusCode);
  headStr_ += " ";
  headStr_ += statusCode2reasonPhrase(statusCode);
  headStr_ += "\r\n";
}

void Response::addHeader(std::string_view name, std::string_view value)
{
  headStr_ += name;
  headStr_ += ": ";
  headStr_ += value;
  headStr_ += "\r\n";
}

void Response::setBody(std::string body)
{
  addHeader("content-length", std::to_string(body.size()));
  bodyStr_ = std::move(body);
  bodyType_ = BodyType::Str;
}

void Response::setBody(std::vector<char> body)
{
  addHeader("content-length", std::to_string(body.size()));
  bodyVec_ = std::move(body);
  bodyType_ = BodyType::Vec;
}

void Response::emplaceBody(std::string_view body)
{
  setBody(std::string(body));
}

void Response::send(int fd)
{
  switch(bodyType_)
  {
    case BodyType::Str:
      send(fd, bodyStr_.data(), bodyStr_.size());
    break;

    case BodyType::Vec:
      send(fd, bodyVec_.data(), bodyVec_.size());
    break;
  }
}

void Response::send(int fd, std::string_view body)
{
  send(fd, body.data(), body.size());
}

void Response::send(int fd, const void *bodyData, size_t bodySize)
{
  write(fd, "\r\n", 2);
  write(fd, headStr_.data(), headStr_.size());
  write(fd, bodyData, bodySize);
}
