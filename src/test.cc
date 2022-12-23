#include "serverloop.hh"
#include "html.hh"
#include "htmlutils.hh"
#include "utils.hh"
#include <filesystem>

using namespace html;
namespace fs = std::filesystem;

int main(int argc, char **argv)
{
  auto loop = ServerLoop(atoi(argv[1]));

  loop.setHandler("/", [](const auto &req)
  {
    if(req.method() != Http::Method::Get)
      return Http::Response(405);

    if(req.uri().size() > 1)
      return Http::Response(404);

    return Http::Response(200)
      .withHeader("content-type", "text/html")
      .withBody(
        hTag("html")
        << hAttr("lang", "en")
        << (hTag("head")
          << (hTag("meta")
            << hAttr("charset", "utf-8"))
          << (hTag("title")
            << hText("Двач 0.1")))
        << (hTag("body")
          << (hTag("h1")
            << hText("Добро пожаловать. Снова."))
          << [&](auto &h) {
            for(auto entry : fs::directory_iterator("posts"))
              if(entry.is_regular_file())
                h << (hTag("p")
                    << hText(getFileAsString(entry.path())));})
        << hDump());
  });

  loop.exec();

  return 0;
}
