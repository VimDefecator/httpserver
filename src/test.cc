#include <iostream>
#include "serverloop.hh"

int main(int argc, char **argv)
{
  auto loop = ServerLoop(atoi(argv[1]));

  loop.addHandler([](const auto &req) -> std::optional<Http::Response>
  {
    if(req.uri() != "/")
      return {};

    if(req.method() != Http::Method::Get)
      return {Http::Response(405)};

    Http::Response res(200);
    res.addHeader("content-type", "text/html");
    res.setBody(
      "<!DOCTYPE html>"
      "<html lang=\"en\">"
      " <head>"
      "  <title>Sample page</title>"
      " </head>"
      " <body>"
      "  <h1>Sample page</h1>"
      "  <p>This is a sample page.</p>"
      "  <!-- this is a comment -->"
      " </body>"
      "</html>");

    return {std::move(res)};
  });

  loop.exec();

  return 0;
}
