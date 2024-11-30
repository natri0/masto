//
// Created by lina on 29.11.24.
//

#pragma once

#include <string>
#include <utility>

/// fwd to not include curl.h in the header
typedef void CURL;

namespace api {

    enum InternalError : long {
        NoBaseUrl = 901,
        NoRegisteredApp = 902,
        NoAuthToken = 903,
    };

    struct Error {
        std::string message;
        long responseCode;

        Error(std::string msg, long resp) : message(std::move(msg)), responseCode(resp) {}

        static Error ok() { return {"", 200}; }

        [[nodiscard]] inline bool isOk() const {
            return responseCode == 200;
        }
    };

    struct AppData {
        CURL *curl;

        std::string baseUrl;
        std::string clientId;
        std::string clientSecret;
        std::string authToken;
    };

    struct Status {
        std::string content;

        // todo: attachments, cw, visibility
    };

    Error createApp(AppData &data);
    std::string getAuthUrl(const AppData &data);
    Error getAuthToken(AppData &data, const std::string &authCode);
    Error getUsername(AppData &data, std::string &username);
    Error postStatus(const AppData &data, const Status &status);
}
