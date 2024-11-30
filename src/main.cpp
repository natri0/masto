#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <filesystem>

#include <raylib.h>
#include <iostream>
#include <curl/curl.h>
#include <csignal>

#include "platform.h"
#include "state.h"
#include "textfield.h"
#include "mastoapi.h"

namespace fs = std::filesystem;

struct State {
    api::AppData api;

    SavedState s;
    enum { post, cfg } screen{post};
    std::string postText;
    std::string username;
    std::string authCode;
};

void drawPostScreen(State &state);
void drawConfigScreen(State &state);

int main() {
    State state;
    if (fs::path path; fs::exists(path = get_config_path())) state.s.fromJSON(read_file(path));

    state.api.curl = curl_easy_init();
    state.api.baseUrl = state.s.base_url;
    state.api.clientId = state.s.client_id;
    state.api.clientSecret = state.s.client_secret;

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(state.s.windowWidth, state.s.windowHeight, "Masto");

    while (!WindowShouldClose() && IsWindowReady()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (!state.s.valid() || state.screen == State::cfg) {
            drawConfigScreen(state);
        } else {
            drawPostScreen(state);
        }

        if (IsWindowReady()) EndDrawing();
    }

    if (IsWindowReady()) CloseWindow();
    return 0;
}

static size_t curl_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t bytes = size * nmemb;

    auto *str = (std::string *) stream;
    str->append((char *) ptr, bytes);

    return bytes;
}

void drawConfigScreen(State &state) {
    static enum { F_baseurl, F_authorizebtn, F_authcode, F_clientid, F_csecret, F_username, F_max } focus = F_baseurl;
    static bool showAuthcode = false;
    static const char *authorizeBtnText = "authorize";

    bool authorizeEnabled = !state.s.base_url.empty() && state.authCode.empty();

    DrawText("base url: ", 4, 8, 10, BLACK);
    DrawText(state.s.base_url.c_str(), 132, 8, 10, BLACK);
    DrawText(authorizeBtnText, state.s.windowWidth - 120, 8, 10, authorizeEnabled ? focus == F_authorizebtn ? DARKBLUE : BLACK : DARKGRAY);

    if (showAuthcode) DrawText("auth code: ", 4, 30, 10, BLACK);
    if (showAuthcode) DrawText(state.authCode.c_str(), 132, 30, 10, BLACK);
    DrawText("client id: ", 4, 52, 10, BLACK);
    DrawText(state.s.client_id.c_str(), 132, 52, 10, BLACK);
    DrawText("client secret: ", 4, 74, 10, BLACK);
    DrawText(state.s.client_secret.c_str(), 132, 74, 10, BLACK);
    DrawText("username: ", 4, 96, 10, BLACK);
    DrawText(state.username.c_str(), 132, 96, 10, BLACK);

    DrawRectangleLines(128, 4, state.s.windowWidth - 256, 18, focus == F_baseurl ? BLUE : BLACK);
    DrawRectangleLines(state.s.windowWidth - 124, 4, 120, 18, authorizeEnabled ? focus == F_authorizebtn ? BLUE : BLACK : GRAY);
    if (showAuthcode) DrawRectangleLines(128, 26, state.s.windowWidth - 132, 18, focus == F_authcode ? BLUE : BLACK);
    DrawRectangleLines(128, 48, state.s.windowWidth - 132, 18, focus == F_clientid ? BLUE : BLACK);
    DrawRectangleLines(128, 70, state.s.windowWidth - 132, 18, focus == F_csecret ? BLUE : BLACK);
    DrawRectangleLines(128, 92, state.s.windowWidth - 132, 18, focus == F_username ? BLUE : BLACK);

    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) focus = (decltype(focus)) ((focus + 1) % F_max);
    if (!authorizeEnabled && focus == F_authorizebtn) focus = (decltype(focus)) (focus+1);
    if (!showAuthcode && focus == F_authcode) focus = (decltype(focus)) (focus+1);

    if (focus == F_authorizebtn && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
        authorizeBtnText = "authorizing...";

        if (state.s.client_id.empty() || state.s.client_secret.empty()) {
            api::Error err = api::createApp(state.api);
            if (!err.isOk()) {
                state.s.base_url = "Failed to register app: " + err.message;
            } else {
                state.s.client_id = state.api.clientId;
                state.s.client_secret = state.api.clientSecret;
            }
        }

        openBrowser(api::getAuthUrl(state.api).c_str());
        showAuthcode = true;
    }

    if (focus == F_authcode && IsKeyPressed(KEY_ENTER)) {
        api::Error err = api::getAuthToken(state.api, state.authCode);
        if (!err.isOk()) {
            state.authCode = "";
            std::cerr << err.message;
        } else {
            state.s.access_token = state.api.authToken;
            authorizeBtnText = "authorized!";

            err = api::getUsername(state.api, state.username);
            if (err.isOk()) {
                auto burl = curl_url();
                curl_url_set(burl, CURLUPART_URL, state.s.base_url.c_str(), 0);
                char *host;
                curl_url_get(burl, CURLUPART_HOST, &host, 0);
                curl_url_cleanup(burl);

                state.username = TextFormat("@%s@%s", state.username.c_str(), host);
                curl_free(host);
            }
        }
    }

    if (focus == F_baseurl) {
        static Tfield tfield;
        tfield.text = &state.s.base_url;

        DoTfield(tfield, state.s.windowWidth - 136);
    }

    if (focus == F_authcode) {
        static Tfield tfield;
        tfield.text = &state.authCode;

        DoTfield(tfield, state.s.windowWidth - 136);
    }

    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_ENTER)) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_C))) {
        state.screen = State::post;
        write_file(get_config_path(), state.s.toJSON());
    }
}

void drawPostScreen(State &state) {
    DrawText(state.s.access_token.empty() ? "not logged in :(" : state.username.c_str(), 8, state.s.windowHeight - 19, 10, BLACK);
    DrawText("Ctrl+Enter to post", 8, state.s.windowHeight - 43, 10, BLACK);
    DrawText("Alt+C to configure", 8, state.s.windowHeight - 67, 10, BLACK);
    DrawText("ESC to quit", 8, state.s.windowHeight - 91, 10, BLACK);

    DrawText("Masto 0.0.1", 8, 9, 10, BLACK);
    DrawText("by @lina@tech.lgbt", 8, 33, 10, BLACK);

    DrawRectangleLines(128, 4, state.s.windowWidth - 132, state.s.windowHeight - 8, BLUE);
    DrawRectangleLines(4, state.s.windowHeight - 24, 120, 20, BLACK);
    DrawRectangleLines(4, state.s.windowHeight - 48, 120, 20, BLACK);
    DrawRectangleLines(4, state.s.windowHeight - 72, 120, 20, BLACK);
    DrawRectangleLines(4, state.s.windowHeight - 96, 120, 20, BLACK);
    DrawRectangleLines(4, 4, 120, 20, BLACK);
    DrawRectangleLines(4, 28, 120, 20, BLACK);

    static Tfield tfield;
    tfield.multiline = true;
    tfield.text = &state.postText;
    DoTfield(tfield, state.s.windowWidth - 136);
    DrawText(state.postText.c_str(), 132, 8, 10, BLACK);

    if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_C)) state.screen = State::cfg;

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_ENTER)) {
        api::Status status;
        status.content = state.postText;
        api::Error err = api::postStatus(state.api, status);
        if (!err.isOk()) {
            state.postText = TextFormat("Failed to post: %s", err.message.c_str());
        } else {
            CloseWindow();
        }
    }
}
