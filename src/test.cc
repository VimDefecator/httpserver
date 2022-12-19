#include <iostream>
#include "serverloop.hh"
#include "html.hh"

int main(int argc, char **argv)
{
  auto loop = ServerLoop(atoi(argv[1]));

  loop.addHandler([](const auto &req) -> std::optional<Http::Response>
  {
    if(req.uri() != "/")
      return {};

    if(req.method() != Http::Method::Get)
      return {Http::Response(405)};

    return {
      Http::Response(200)
        .addHeader("content-type", "text/html")
        .setBody(
          Html("html", {{"lang", "en"}})
            .addChild(Html("head", {})
              .addChild(Html("title", {})
                .addChild(Html("Sample page"))
                .move())
              .move())
            .addChild(Html("body", {})
              .addChild(Html("h1", {})
                .addChild(Html("Sample page"))
                .move())
              .addChild(Html("p", {})
                .addChild(Html("This is a sample page"))
                .move())
              .move())
            .dump())
        .move()};
  });

  loop.exec();

  return 0;
}
