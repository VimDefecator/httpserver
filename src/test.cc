#include <iostream>
#include <charconv>
#include "serverloop.hh"
#include "html.hh"
#include "utils.hh"

int main(int argc, char **argv)
{
  auto loop = ServerLoop(atoi(argv[1]));

  loop.setHandler("/", [](const auto &req)
  {
    if(req.method() != Http::Method::Get)
      return Http::Response(405);

    return Http::Response(200)
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
              .addChild(Html("Sample header"))
              .move())
            .applyFn([uri=req.uri()](auto &h)
            {
              auto p = Html("p", {})
                        .addChild(Html("Sample paragraph"))
                        .move();

              for(auto num = str2num<int>(uri.substr(1)); num--;)
                h.addChild(p);
            })
            .move())
          .dump())
      .move();
  });

  loop.exec();

  return 0;
}
