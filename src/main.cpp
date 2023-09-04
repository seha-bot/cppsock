#include "http.hpp"
#include <iostream>

int main() {
    Router router;
    router.serve_directory("www");

    std::vector<std::string> messages;

    router.get("/messages", [&messages](HttpRequest req) {
        std::string res;
        for (const auto& msg : messages) {
            res.append("<p>");
            res.append(msg);
            res.append("</p>");
        }
        return HttpResponse(200, res);
    });

    router.post("/messages", [&messages](HttpRequest req) {
        std::string str(req.bytes.begin(), req.bytes.end());
        messages.push_back(str);
        return HttpResponse(200);
    });

    router.run(8080);
    return 0;
}
