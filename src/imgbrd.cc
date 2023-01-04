#include "imgbrd.hh"
#include "serverloop.hh"
#include "html.hh"
#include "htmlutils.hh"
#include "utils.hh"

#include <atomic>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <cstdlib>

namespace fs = std::filesystem;
using namespace html;

static constexpr auto c_pageSize = 20;

namespace
{
  Html makePostFromFileTrunc(const std::string &filename)
  {
    auto path = "posts/" + filename;

    auto buf = std::string(0x100, '\0');
    auto tooBig = false;
    {
      auto file = std::ifstream(path);
      file.read(buf.data(), buf.size());
      if(file.gcount() < buf.size())
        buf.resize(file.gcount());
      else
        tooBig = file.seekg(0, std::ios::end).tellg() > buf.size();
    }

    if(tooBig)
    {
      if(uint8_t(buf.back()) >= 0x80)
      {
        while(uint8_t(buf.back()) <= 0xbf)
          buf.pop_back();

        buf.pop_back();
      }

      buf += "...";
    }

    return
      hTag("p")
      << (hTag("h3") << hText(filename))
      << (hTag("pre")
        << hText(buf)
        << (tooBig ? (hTag("a").wAttr("href", "/post/" + filename) << hText("MORE")) : hNop()));
  }

  Html makePostFromFile(const std::string &filename)
  {
    return
      hTag("p")
      << (hTag("h3") << hText(filename))
      << (hTag("pre") << hText(getFileAsString("posts/" + filename)));
  }

  Html makePostingForm()
  {
    return
      hTag("form")
      .wAttr("method", "post")
      .wAttr("enctype", "text/plain")
      .wAttr("action", "/post")
      << (hTag("p")
        << hTag("textarea")
          .wAttr("name", "content")
          .wAttr("rows", "6")
          .wAttr("cols", "60"))
      << (hTag("p")
        << (hTag("button") << hText("Отправить")));
  }

  Html makePageLink(unsigned pageNo, std::string text)
  {
    return
      hTag("a").wAttr("href", "/page/" + std::to_string(pageNo))
      << hText(std::move(text));
  }

  Html makePageLink(unsigned pageNo)
  {
    return makePageLink(pageNo, std::to_string(pageNo));
  }

  Html makePaginationPanel(unsigned pageNo, unsigned numPages)
  {
    return
      hTag("p")
      << (pageNo > 0
        ? makePageLink(pageNo - 1, "<-пред ")
        : hText("<-пред "))
      << hText("|")
      << (pageNo < numPages - 1
        ? makePageLink(pageNo + 1, " след->")
        : hText(" след->"))
      << hTag("br").wNoClose()
      << makePageLink(0)
      << [&](auto &h) {
        auto fromPageNo = pageNo > 4 ? pageNo - 3 : 1;
        auto toPageNo = std::min(pageNo + 3, numPages - 2);
        if(fromPageNo > 1)
          h << hText("...");
        for(auto no = fromPageNo; no <= toPageNo; ++no)
          h << makePageLink(no);
        if(toPageNo < numPages - 2)
          h << hText("...");}
      << makePageLink(numPages - 1);
  }

  Html makePage(Html body)
  {
    return
      hTag("html").wAttr("lang", "en")
      << (hTag("head")
        << (hTag("meta")
          << hAttr("charset", "utf-8"))
        << (hTag("title")
          << hText("Двач 0.1")))
      << body.move();
  }

  Http::Response makeRedirect(std::string_view uri)
  {
    return Http::Response(303).withHeader("location", uri);
  }
}

struct ImgBrd::Impl
{
  ServerLoop serverLoop_;
  std::atomic<unsigned> numPosts_;
  int port_;

  Http::Response handleGetMain(const Http::Request &req, std::string_view subURI);
  Http::Response handleGetPage(const Http::Request &req, std::string_view subURI);
  Http::Response handleGetPost(const Http::Request &req, std::string_view subURI);
  Http::Response handlePostPost(const Http::Request &req, std::string_view subURI);

  using Handler = Http::Response (Impl::*)(const Http::Request &, std::string_view);
  void setHandler(Http::Method method, std::string uri, Handler handler);
};

Http::Response ImgBrd::Impl::handleGetMain(const Http::Request &req, std::string_view subURI)
{
  if(!subURI.empty())
    return Http::Response(404);

  return makeRedirect("/page/0");
}

Http::Response ImgBrd::Impl::handleGetPage(const Http::Request &req, std::string_view subURI)
{
  auto pageNo = str2num<unsigned>(subURI);

  auto numPosts = numPosts_.load();
  auto numPages = numPosts ? (numPosts - 1) / c_pageSize + 1 : 1;

  if(pageNo > numPages)
    return Http::Response(404);

  return Http::Response(200)
    .withHeader("content-type", "text/html")
    .withBody(
      makePage(
        hTag("body")
        << (hTag("h1")
          << hText("Добро пожаловать. Снова."))
        << makePaginationPanel(pageNo, numPages)
        << [&](auto &h) {
          auto fromPostNo = pageNo * c_pageSize;
          auto toPostNo = std::min(numPosts, fromPostNo + c_pageSize);
          for(unsigned postNo = fromPostNo; postNo < toPostNo; ++postNo) {
            auto filename = std::to_string(postNo);
            h << makePostFromFileTrunc(filename).wAttr("id", "post" + filename);}}
        << makePaginationPanel(pageNo, numPages)
        << makePostingForm())
      << hDump());
}

Http::Response ImgBrd::Impl::handleGetPost(const Http::Request &req, std::string_view subURI)
{
  auto filename = std::string(subURI);

  return Http::Response(200)
    .withHeader("content-type", "text/html")
    .withBody(
      makePage(
        hTag("body")
        << makePostFromFile(filename))
      << hDump());
}

Http::Response ImgBrd::Impl::handlePostPost(const Http::Request &req, std::string_view subURI)
{
  if(!subURI.empty())
    return Http::Response(404);

  auto numPosts = numPosts_++;

  auto filename = std::to_string(numPosts);
  {
    auto file = std::ofstream("posts/" + filename);
    file << req.bodyStr().substr(8);
  }

  return makeRedirect("/page/" + std::to_string(numPosts / c_pageSize) + "/#post" + filename);
}

void ImgBrd::Impl::setHandler(Http::Method method, std::string uri, Handler handler)
{
  auto lenBaseURI = uri.size();
  serverLoop_.setHandler(method, std::move(uri),
    [this, handler, lenBaseURI](const auto &req) {
      return ((*this).*handler)(req, req.uri().substr(lenBaseURI)); });
}

ImgBrd::ImgBrd()
{
}

ImgBrd::~ImgBrd()
{
}

void ImgBrd::init(int argc, char **argv)
{
  impl_.reset(new Impl);

  impl_->port_ = argc >= 2 ? atoi(argv[1]) : 80;

  impl_->numPosts_ = std::count_if(fs::directory_iterator("posts"),
                                   fs::directory_iterator(),
                                   [](auto &ent){return ent.is_regular_file();});

  {
    using M = Http::Method;
    impl_->setHandler(M::Get , "/"     , &Impl::handleGetMain);
    impl_->setHandler(M::Get , "/page/", &Impl::handleGetPage);
    impl_->setHandler(M::Get , "/post/", &Impl::handleGetPost);
    impl_->setHandler(M::Post, "/post" , &Impl::handlePostPost);
  }

  ServerLoop::initSignalHandling();
}

void ImgBrd::exec()
{
  impl_->serverLoop_.exec(impl_->port_, 4);
}
