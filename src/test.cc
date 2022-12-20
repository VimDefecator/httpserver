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

    if(req.uri().size() > 1)
      return Http::Response(404);

    return Http::Response(200)
      .withHeader("content-type", "text/html")
      .withBody(
        Html("html", {{"lang", "en"}})
          .withChild(Html("head", {})
            .withChild(Html("title", {})
              .withChild(Html("Sashka website"))))
          .withChild(Html("body", {})
            .withChild(Html("h1", {})
              .withChild(Html("Welcome!")))
            .withChild(Html("p", {})
              .withChild(Html("HERE'S SOME PARAGRAPH FOR YOU MAN"))))
          .dump());
  });

  loop.exec();

  return 0;
}
