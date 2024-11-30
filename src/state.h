//
// Created by lina on 28.11.24.
//

#pragma once

#include <string>
#include <nlohmann/json.hpp>

struct SavedState {
    std::string base_url;
    std::string client_id;
    std::string client_secret;

    std::string access_token;

    int windowWidth = 640;
    int windowHeight = 480;

    [[nodiscard]] bool valid() const {
        return !base_url.empty() && !client_id.empty() && !client_secret.empty();
    }

    [[nodiscard]] std::string toJSON() const {
        nlohmann::json json;

        json["base_url"] = base_url;
        json["client_id"] = client_id;
        json["client_secret"] = client_secret;
        json["access_token"] = access_token;
        json["windowWidth"] = windowWidth;
        json["windowHeight"] = windowHeight;

        return json.dump();
    }

    void fromJSON(const std::string& json_str) {
        auto json = nlohmann::json::parse(json_str);

#define Deser(nm) if (json.contains(#nm)) nm = json[#nm]
        Deser(base_url);
        Deser(client_id);
        Deser(client_secret);
        Deser(access_token);
        Deser(windowWidth);
        Deser(windowHeight);
#undef  Deser
    }
};
