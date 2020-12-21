#pragma once

#include <vector>

#include <QWidget>
// move Qt headers (from .cpp) before <X11/Xlib.h>, so they don't conflict
#include <QPainter>
#include <QTimer>
#include <QGuiApplication>
#include <QWindow>
#include <QScreen>
#include <QDebug>

#include <X11/Xlib.h>

using std::vector;

struct Solution {
    int i1, j1;
    int i2, j2;
    bool operator==(const Solution& rhs) const {
        return i1 == rhs.i1 && j1 == rhs.j1 && i2 == rhs.i2 && j2 == rhs.j2;
    }
    bool operator<(const Solution& rhs) const {
        return i1 < rhs.i1 || i2 < rhs.i2 || j1 < rhs.j1 || j2 < rhs.j2;
    }
};

class Overlay : public QWidget
{
    Q_OBJECT

    const char* STARCRAFT_WINDOW_NAME = "星际争霸";
    const int N = 8, PADDING = 3;
    // board geometry for resolution 1920x1080
    const int w = 72, x0 = 1305, y0 = 101;

    vector<vector<int>> board;

    Display* display;
    Window root_win;

public:
    explicit Overlay(QWidget *parent = 0);
    ~Overlay();

protected:
    void paintEvent(QPaintEvent *) override;
    XImage* shootScreen(Window window);

protected slots:
    void make_move();

private:
    Window get_window(const char* name);
    int match_color(const vector<int>& c, vector<vector<int>>& palette, int threshold = 10);
    void get_board(XImage* img, QPainter& p);
    bool is_solution(const int m, const int n, const int dx, const int dy);
    vector<Solution> calc_solutions();
};
