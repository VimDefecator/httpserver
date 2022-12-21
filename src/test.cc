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
        hTag("html").wAttr("lang", "en")
          << (hTag("head")
            << (hTag("title")
              << hText("Sashka website")))
          << (hTag("body")
            << (hTag("h1")
              << hText("Welcome!"))
            << (hTag("form")
              << (hTag("p")
                << (hTag("label")
                  << hText("Customer name:")
                  << hTag("input").wNoClose()))
              << (hTag("fieldset")
                << (hTag("legend") << hText("Pizza size"))
                << (hTag("p")
                  << (hTag("label")
                    << hTag("input").wAttr("type", "radio").wAttr("name", "size").wNoClose()
                    << hText("Small"))
                  << (hTag("label")
                    << hTag("input").wAttr("type", "radio").wAttr("name", "size").wNoClose()
                    << hText("Medium"))
                  << (hTag("label")
                    << hTag("input").wAttr("type", "radio").wAttr("name", "size").wNoClose()
                    << hText("Large"))))))
          << Html::Dump());
  });

  loop.exec();

  return 0;
}
