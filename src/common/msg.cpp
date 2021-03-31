#include "msg.h"

#include <spdlog/spdlog.h>

std::optional<MessageResults> msg(cpr::Session& sess, const std::string& fqmn, std::vector<cpr::Pair> data)
{
    data.emplace_back("msg", fqmn);
    sess.SetPayload(cpr::Payload{data.begin(), data.end()});

    const auto r = sess.Post();

    if (r.status_code != 200) {
        if (r.status_code > 0)
            spdlog::get("combined")->warn(r.status_line);
        else
            spdlog::get("combined")->error(r.error.message);

        return {};
    }

    const auto json = nlohmann::json::parse(r.text);

    if (json["status"] != 0) {
        if (json["status"] > 0)
            spdlog::get("combined")->warn("{} ({})", json["status_msg"], json["status"]);
        else
            spdlog::get("combined")->error("{} ({})", json["status_msg"], json["status"]);
    }

    return {MessageResults{json["status"], 1000.0f * static_cast<float>(r.elapsed), json}};
}

cpr::Session msg_login(const std::string& url, const std::string& user, const std::string& password, std::chrono::milliseconds timeout)
{
    spdlog::info("login...");

    cpr::Session sess;
    sess.SetUrl(url + "/cmd.php");
    sess.SetTimeout(timeout);

    const auto res = msg(sess, "login.login", {{"login", user}, {"pwd", password}});

    if (!res.has_value() || res->status != 0) {
        spdlog::error("login failed");
        std::exit(2);
    }

    return sess;
}

void msg_logout(cpr::Session& sess)
{
    spdlog::info("logout...");

    msg(sess, "login.logout", {});
}
