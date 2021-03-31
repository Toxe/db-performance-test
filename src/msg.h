#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

struct MessageResults {
    int status;
    float elapsed;
    nlohmann::json json;
};

std::optional<MessageResults> msg(cpr::Session& sess, const std::string& fqmn, std::vector<cpr::Pair> data);
cpr::Session msg_login(const std::string& url, const std::string& user, const std::string& password, std::chrono::milliseconds timeout);
void msg_logout(cpr::Session& sess);
