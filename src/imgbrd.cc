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

namespace
{
  Html makePostFromFileTrunc(const std::string &path)
  {
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

    return hTag("p")
      << (hTag("h3") << hText(path))
      << (hTag("pre")
        << hText(buf)
        << (tooBig ? (hTag("a") << hAttr("href", path) << hText("MORE")) : hNop()));
  }

  Html makePostFromFile(const std::string &path)
  {
    return hTag("p")
      << (hTag("h3") << hText(path))
      << (hTag("pre") << hText(getFileAsString(path)));
  }

  Html makePostingForm()
  {
    return hTag("form")
      << hAttr("method", "post")
      << hAttr("enctype", "text/plain")
      << hAttr("action", "/posts")
      << (hTag("p")
        << hTag("textarea").wAttr("name", "content").wAttr("rows", "6").wAttr("cols", "60"))
      << (hTag("p")
        << (hTag("button") << hText("Отправить")));
  }

  Html makePage(Html body)
  {
    return hTag("html")
      << hAttr("lang", "en")
      << (hTag("head")
        << (hTag("meta")
          << hAttr("charset", "utf-8"))
        << (hTag("title")
          << hText("Двач 0.1")))
      << body.move();
  }
}

struct ImgBrd::State
{
  ServerLoop serverLoop;
  std::atomic<unsigned> numPosts;
};

ImgBrd::ImgBrd()
{
}

ImgBrd::~ImgBrd()
{
}

void ImgBrd::init(int argc, char **argv)
{
  auto port = argc >= 2 ? atoi(argv[1]) : 80;

  state_.reset(new State{.serverLoop = ServerLoop(port)});

  state_->numPosts = std::count_if(fs::directory_iterator("posts"),
                                   fs::directory_iterator(),
                                   [](auto &ent){return ent.is_regular_file();});

  state_->serverLoop.setHandler("/", [state = state_.get()](const auto &req)
  {
    if(req.uri().size() > 1)
      return Http::Response(404);

    if(req.method() == Http::Method::Get)
    {
      return Http::Response(200)
        .withHeader("content-type", "text/html")
        .withBody(
          makePage(
            hTag("body")
            << (hTag("h1")
              << hText("Добро пожаловать. Снова."))
            << [&](auto &h) {
              auto numPosts = state->numPosts.load();
              for(unsigned postNo = 0; postNo < numPosts; ++postNo)
                h << makePostFromFileTrunc("posts/" + std::to_string(postNo));}
            << makePostingForm())
          << hDump());
    }

    return Http::Response(405);
  });

  state_->serverLoop.setHandler("/posts", [state = state_.get()](const auto &req)
  {
    if(req.method() == Http::Method::Get)
    {
      auto path = std::string(req.uri().substr(1));

      return Http::Response(200)
        .withHeader("content-type", "text/html")
        .withBody(
          makePage(
            hTag("body")
            << makePostFromFile(path))
          << hDump());
    }

    if(req.method() == Http::Method::Post)
    {
      {
        auto file = std::ofstream("posts/" + std::to_string(state->numPosts++));
        file << req.bodyStr().substr(8);
      }

      return Http::Response(303)
        .withHeader("location", "/");
    }

    return Http::Response(405);
  });
}

void ImgBrd::exec()
{
  state_->serverLoop.exec();
}
