#pragma once
#include <string>

#include "x.hpp"

class ScreenCapture {
private:
    Display *display;
    Window root_win;
    XImage* img;
    X11 x11;

public:
    ScreenCapture();
    ~ScreenCapture();

    bool capture(const std::string &window_title, const int x, const int y, const int width, const int height);
    unsigned long get_pixel(int x, int y);
    bool is_window_visible(const std::string &title, const int min_width = -1, const int min_height = -1);
};
