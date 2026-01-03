#include <drogon/drogon.h>

int main() {
    drogon::app()
        .addListener("0.0.0.0", 8080)
        .registerHandler("/health",
            [](const drogon::HttpRequestPtr&,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
                    auto resp = drogon::HttpResponse::newHttpResponse();
                    resp->setStatusCode(drogon::k200OK);
                    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
                    resp->setBody("ok\n");
                    callback(resp);
            },
            { drogon::Get })
        .run();
}
