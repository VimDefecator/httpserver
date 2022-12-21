#include "serverloop.hh"
#include "html.hh"
#include "htmlutils.hh"
#include "utils.hh"

using namespace html;

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
            << hFieldSet("Pizza size",
                        {{"type", "radio"}, {"name", "size"}},
                        {"Small", "Medium", "Large"})
            << hFieldSet("Pizza toppings",
                        {{"type", "checkbox"}},
                        {"Bacon", "Extra cheese", "Onion", "Mushrom"})))
        << hDump());
  });

  loop.exec();

  return 0;
}
