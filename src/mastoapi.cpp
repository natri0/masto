//
// Created by lina on 29.11.24.
//

#include "mastoapi.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// fwd from raylib.h
extern "C" const char *TextFormat(const char *text, ...);

static size_t curl_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t bytes = size * nmemb;

    auto *str = (std::string *) stream;
    str->append((char *) ptr, bytes);

    return bytes;
}

api::Error api::createApp(api::AppData &data) {
    std::string content;

    curl_easy_setopt(data.curl, CURLOPT_URL, TextFormat("%s/api/v1/apps", data.baseUrl.c_str()));
    curl_easy_setopt(data.curl, CURLOPT_POST, 1L);
    curl_easy_setopt(data.curl, CURLOPT_POSTFIELDS, "client_name=Masto&redirect_uris=urn:ietf:wg:oauth:2.0:oob&scopes=profile write:statuses&website=");
    curl_easy_setopt(data.curl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(data.curl, CURLOPT_WRITEDATA, &content);

    curl_easy_perform(data.curl);

    long resp_code;
    curl_easy_getinfo(data.curl, CURLINFO_RESPONSE_CODE, &resp_code);

    auto json = json::parse(content);

    if (resp_code != 200) {
        return {json["error"], resp_code};
    } else {
        auto json_resp = json::parse(content);
        data.clientId = json_resp["client_id"];
        data.clientSecret = json_resp["client_secret"];
    }

    return Error::ok();
}

std::string api::getAuthUrl(const api::AppData &data) {
    return {TextFormat("%s/oauth/authorize?client_id=%s&redirect_uri=urn:ietf:wg:oauth:2.0:oob&response_type=code&scope=profile%%20write:statuses", data.baseUrl.c_str(), data.clientId.c_str())};
}

api::Error api::getAuthToken(api::AppData &data, const std::string &authCode) {
    std::string content;

    curl_easy_setopt(data.curl, CURLOPT_URL, TextFormat("%s/oauth/token", data.baseUrl.c_str()));
    curl_easy_setopt(data.curl, CURLOPT_POST, 1L);
    curl_easy_setopt(data.curl, CURLOPT_POSTFIELDS, TextFormat("client_id=%s&client_secret=%s&grant_type=authorization_code&code=%s&redirect_uri=urn:ietf:wg:oauth:2.0:oob", data.clientId.c_str(), data.clientSecret.c_str(), authCode.c_str()));
    curl_easy_setopt(data.curl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(data.curl, CURLOPT_WRITEDATA, &content);
    curl_easy_perform(data.curl);

    long resp_code;
    curl_easy_getinfo(data.curl, CURLINFO_RESPONSE_CODE, &resp_code);

    auto json = json::parse(content);

    if (resp_code != 200) {
        return {content, resp_code};
    } else {
        auto json_resp = json::parse(content);
        data.authToken = json_resp["access_token"];
    }

    return Error::ok();
}

api::Error api::postStatus(const api::AppData &data, const api::Status &status) {
    std::string content;

    curl_easy_setopt(data.curl, CURLOPT_URL, TextFormat("%s/api/v1/statuses", data.baseUrl.c_str()));
    curl_easy_setopt(data.curl, CURLOPT_POST, 1L);
    curl_easy_setopt(data.curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    curl_easy_setopt(data.curl, CURLOPT_XOAUTH2_BEARER, data.authToken.c_str());
    curl_easy_setopt(data.curl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(data.curl, CURLOPT_WRITEDATA, &content);

    auto *fields = curl_mime_init(data.curl);
    auto *field = curl_mime_addpart(fields);
    curl_mime_name(field, "status");
    curl_mime_data(field, status.content.c_str(), CURL_ZERO_TERMINATED);
    curl_easy_setopt(data.curl, CURLOPT_MIMEPOST, fields);
    curl_easy_perform(data.curl);

    long resp_code;
    curl_easy_getinfo(data.curl, CURLINFO_RESPONSE_CODE, &resp_code);

    if (resp_code != 200) {
        return {content, resp_code};
    }

    return Error::ok();
}

api::Error api::getUsername(api::AppData &data, std::string &username) {
    std::string content;

    curl_easy_setopt(data.curl, CURLOPT_URL, TextFormat("%s/api/v1/accounts/verify_credentials", data.baseUrl.c_str()));
    curl_easy_setopt(data.curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(data.curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    curl_easy_setopt(data.curl, CURLOPT_XOAUTH2_BEARER, data.authToken.c_str());
    curl_easy_setopt(data.curl, CURLOPT_WRITEFUNCTION, curl_cb);
    curl_easy_setopt(data.curl, CURLOPT_WRITEDATA, &content);
    curl_easy_perform(data.curl);

    long resp_code;
    curl_easy_getinfo(data.curl, CURLINFO_RESPONSE_CODE, &resp_code);

    auto json = json::parse(content);

    if (resp_code != 200) {
        return {json["error"], resp_code};
    } else {
        username = json["username"];
    }

    return Error::ok();
}
