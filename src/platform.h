//
// Created by lina on 28.11.24.
//

#pragma once

#include <string>
#include <filesystem>

std::string read_file(const std::filesystem::path &path);
void write_file(const std::filesystem::path &path, const std::string &content);
std::filesystem::path get_config_path();

void openBrowser(const char *url);
