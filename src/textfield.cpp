//
// Created by lina on 28.11.24.
//

#include "textfield.h"
#include <raylib.h>
#include <cstring>

void insert(Tfield &tf, int n, char const *c) {
    if (tf.csrL == -1) tf.text->insert(tf.csrR, c, n);
    else tf.text->replace(tf.text->begin() + tf.csrL, tf.text->begin() + tf.csrR, c, n);
    tf.csrR = (tf.csrL == -1 ? tf.csrR : tf.csrL) + n;
    tf.csrL = -1;
}

void DoTfield(Tfield &tf, int maxWidth) {
    if (!tf.text) return;

    if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && tf.csrR < tf.text->length()) {
        if (!IsKeyDown(KEY_LEFT_SHIFT) && tf.csrL != -1) tf.csrL = -1;
        else {
            if (IsKeyDown(KEY_LEFT_SHIFT) && tf.csrL == -1) tf.csrL = tf.csrR;
            tf.csrR++;
        }
    }

    if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && tf.csrL != 0) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) tf.csrL = ((tf.csrL == -1) ? tf.csrR : tf.csrL) - 1;
        else if (tf.csrL == -1) tf.csrR--;
        else { tf.csrR = tf.csrL; tf.csrL = -1; }
    }

    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && tf.csrR) tf.text->erase(--tf.csrR, 1);
    if ((IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE))) tf.text->erase(tf.csrR + 1, 1);

    for (int c; (c = GetCharPressed()) && MeasureText(tf.text->c_str(), 10) < maxWidth;) {
        insert(tf, 1, (char const *) &c);
    }

    if (const char *paste; IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V) && (paste = GetClipboardText())) {
        insert(tf, strlen(paste), paste);
    }

    if (IsKeyPressed(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
        tf.csrL = 0;
        tf.csrR = tf.text->length();
    }

    if (tf.multiline && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))) {
        insert(tf, 1, "\n");
    }
}
