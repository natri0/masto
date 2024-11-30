//
// Created by lina on 28.11.24.
//

#pragma once

#include <string>

struct Tfield {
    bool multiline = false;
    int csrL = -1, csrR = 0;
    std::string *text = nullptr;
};

void DoTfield(Tfield &tf, int maxWidth);
