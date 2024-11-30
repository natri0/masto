//
// Created by lina on 28.11.24.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "platform.h"

#include <dbus/dbus.h>

namespace fs = std::filesystem;

std::string read_file(const fs::path &path) {
    std::ifstream file(path);
    return {std::istreambuf_iterator<char>(file), {}};
}

void write_file(const fs::path &path, const std::string &content) {
    std::ofstream file(path);
    file.write(content.data(), content.size());
    file.close();
}

fs::path get_config_path() {
    if (const char *xdg_config_home = getenv("XDG_CONFIG_HOME")) {
        return fs::path(xdg_config_home) / "masto.json";
    } else {
        return fs::path(getenv("HOME")) / ".config" / "masto.json";
    }
}

void openBrowser(const char *url) {
    // the same implementation as in main_old.cpp:

    static DBusConnection *conn;
    static DBusError err;

    static bool inited = false;

    if (!inited) {
        dbus_error_init(&err);
        conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (dbus_error_is_set(&err)) {
            std::cerr << "Failed to connect to the D-Bus session bus: " << err.message << std::endl;
            dbus_error_free(&err);
            return;
        }

        inited = true;
    }

    DBusMessage *msg;
    msg = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.OpenURI", "OpenURI");

    if (!msg) {
        std::cerr << "Failed to create a D-Bus message" << std::endl;
        return;
    }

    DBusMessageIter args, dict;
    dbus_message_iter_init_append(msg, &args);

    const char *appid = "masto";

    if (!dbus_message_iter_append_basic(&args, 's', &appid) ||
        !dbus_message_iter_append_basic(&args, 's', &url) ||
        !dbus_message_iter_open_container(&args, 'a', "{sv}", &dict) ||
        !dbus_message_iter_close_container(&args, &dict)) {
        std::cerr << "Failed to append arguments to the D-Bus message" << std::endl;
        return;
    }

    DBusPendingCall *pending;
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1) || !pending) {
        std::cerr << "Failed to send the D-Bus message" << std::endl;
        return;
    }

    dbus_connection_flush(conn);
    dbus_message_unref(msg);
    dbus_pending_call_block(pending);
    dbus_pending_call_unref(pending);
}
