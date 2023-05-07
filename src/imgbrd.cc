#include "imgbrd.hh"
#include "serverloop.hh"
#include "html.hh"
#include "htmlutils.hh"
#include "utils.hh"
#include "threadsafeobject.hh"
#include "args.hh"

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
        << hIf(tooBig, [&](auto &h) {
          h << (hTag("a").wAttr("href", "/post/" + filename) << hText("MORE")); }));
  }

  Html makePage(Html body)
  {
    return
      hTag("html")
      .wAttr("lang", "en")
      << (hTag("head")
        << hTag("meta")
          .wAttr("charset", "utf-8")
        << (hTag("title")
          << hText("Двач 0.1")))
      << (hTag("body")
        << body.move());
  }

  Html makeHeading()
  {
    return hMany()
      << hTag("img").wAttr("src", "/pics/main.jpg").wNoClose()
      << hTag("br").wNoClose()
      << (hTag("h1") << hText("Добро пожаловать. Снова."));
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

  Html makePosts(unsigned pageNo, unsigned numPosts)
  {
    return hMany() << [&](auto &h)
    {
      auto fromPostNo = pageNo * c_pageSize;
      auto toPostNo = std::min(numPosts, fromPostNo + c_pageSize);
      for(unsigned postNo = fromPostNo; postNo < toPostNo; ++postNo)
      {
        auto filename = std::to_string(postNo);
        h << makePostFromFileTrunc(filename).wAttr("id", "post" + filename);
      }
    };
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

  Http::Response makeRedirect(std::string_view uri)
  {
    return Http::Response(303)
      .withHeader("location", uri);
  }

  Http::Response makeContent200(std::string_view content)
  {
    return Http::Response(200)
      .withHeader("content-type", "text/html")
      .withBody(content);
  }
}

struct ImgBrd::Impl
{
  ServerLoop serverLoop_;
  int port_ = 80;
  std::atomic<unsigned> numPosts_;
  ThreadSafeObject<std::map<std::string, std::string, std::less<>>> cache_;

  template<typename Result>
  Result useCache(std::string_view uri,
                  std::function<std::string()> makeContent,
                  std::function<Result(std::string_view)> useContent);

  Http::Response handleGetMain(const Http::Request &req, std::string_view subURI);
  Http::Response handleGetPage(const Http::Request &req, std::string_view subURI);
  Http::Response handleGetPost(const Http::Request &req, std::string_view subURI);
  Http::Response handleGetPic(const Http::Request &req, std::string_view subURI);
  Http::Response handlePostPost(const Http::Request &req, std::string_view subURI);

  using Handler = Http::Response (Impl::*)(const Http::Request &, std::string_view);
  void setHandler(Http::Method method, std::string uri, Handler handler);
};

template<typename Result>
Result ImgBrd::Impl::useCache(std::string_view uri,
                              std::function<std::string()> makeContent,
                              std::function<Result(std::string_view)> useContent)
{
  {
    std::shared_lock _(cache_.mx());

    if(auto it = cache_->find(uri); it != cache_->end())
      return useContent(it->second);
  }

  std::unique_lock _(cache_.mx());

  auto [it, isNew] = cache_->emplace(uri, "");
  auto &content = it->second;

  if(isNew)
    content = makeContent();

  return useContent(content);
}

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

  if(pageNo < numPages)
    return useCache<Http::Response>(req.uri(),
      [&] {
        return makePage(
          hMany()
          << makeHeading()
          << makePaginationPanel(pageNo, numPages)
          << makePosts(pageNo, numPosts)
          << makePaginationPanel(pageNo, numPages)
          << makePostingForm()
        ).dump();
      },
      makeContent200);
  else
    return Http::Response(404);
}

Http::Response ImgBrd::Impl::handleGetPost(const Http::Request &req, std::string_view subURI)
{
  auto filename = std::string(subURI);

  if(auto content = getFileAs<std::string>("posts/" + filename))
    return Http::Response(200)
      .withHeader("content-type", "text/html")
      .withBody(
        makePage(
          hTag("p")
          << (hTag("h3") << hText(filename))
          << (hTag("pre") << hText(*content))).dump());
  else
    return Http::Response(404);
}

Http::Response ImgBrd::Impl::handleGetPic(const Http::Request &req, std::string_view subURI)
{
  auto filename = std::string(subURI);

  if(auto content = getFileAs<std::vector<char>>("pics/" + filename))
    return Http::Response(200)
      .withHeader("content-type", "image/jpeg")
      .withBody(std::move(*content));
  else
    return Http::Response(404);
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

  auto lastPageUri = "/page/" + std::to_string(numPosts / c_pageSize);

  {
    std::unique_lock _(cache_.mx());

    if(numPosts % c_pageSize == 0)
    {
      auto it1 = cache_->lower_bound("/page/");
      auto it2 = it1;

      while(it2 != cache_->end() && it2->second.starts_with("/page/"))
        ++it2;

      cache_->erase(it1, it2);
    }
    else
    {
      cache_->erase(lastPageUri);
      cache_->erase(lastPageUri + "/");
    }
  }

  return makeRedirect(lastPageUri + "/#post" + filename);
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

void ImgBrd::init(const Args &args)
{
  impl_.reset(new Impl);

  impl_->port_ = args.getIntO("port").value_or(80);

  impl_->numPosts_ = std::count_if(fs::directory_iterator("posts"),
                                   fs::directory_iterator(),
                                   [](auto &ent){return ent.is_regular_file();});

  {
    using M = Http::Method;
    impl_->setHandler(M::Get , "/"     , &Impl::handleGetMain);
    impl_->setHandler(M::Get , "/page/", &Impl::handleGetPage);
    impl_->setHandler(M::Get , "/post/", &Impl::handleGetPost);
    impl_->setHandler(M::Get , "/pics/", &Impl::handleGetPic);
    impl_->setHandler(M::Post, "/post" , &Impl::handlePostPost);
  }

  ServerLoop::initSignalHandling();
}

void ImgBrd::exec()
{
  impl_->serverLoop_.exec(impl_->port_, 4);
}
