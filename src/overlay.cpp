#include "overlay.h"

#include <unistd.h>

#include "x.hpp"
#include "mouse.h"

int is_window_visible(Display *display, Window wid) {
    XWindowAttributes wattr;
    XGetWindowAttributes(display, wid, &wattr);
    if (wattr.map_state != IsViewable)
        return False;

    return True;
}

Window window_from_name_search(Display *display, Window current, char const *target) {
    /* Check if this window has the name we seek */
    XTextProperty text;
    if(XGetWMName(display, current, &text) > 0 && text.nitems > 0) {
        int count = 0;
        char **list = NULL;
        if (Xutf8TextPropertyToTextList(display, &text, &list, &count) == Success && count > 0) {
            const char* r = strstr(list[0], target);
            if(r != NULL) {
                XFree(text.value);
                XFreeStringList(list);
                return current;
            }
        }
        XFreeStringList(list);
    }
    XFree(text.value);

    Window retval = 0, root, parent, *children;
    unsigned children_count;

    /* If it does not: check all subwindows recursively. */
    if(0 != XQueryTree(display, current, &root, &parent, &children, &children_count)) {
        for(unsigned i = 0; i < children_count; ++i) {
            Window win = window_from_name_search(display, children[i], target);

            if(win != 0) {
                retval = win;
                break;
            }
        }

        XFree(children);
    }

    return retval;
}

std::pair<int, int> get_mouse_position(Display* dpy) {
    int ret = False;
    std::pair<int, int> r{-1, -1};
    Window window = 0, root = 0;
    int dummy_int = 0;
    unsigned int dummy_uint = 0;

    for (int i = 0; i < ScreenCount(dpy); i++) {
        Screen *screen = ScreenOfDisplay(dpy, i);
        ret = XQueryPointer(dpy, RootWindowOfScreen(screen), &root, &window, &r.first, &r.second, &dummy_int, &dummy_int,
                            &dummy_uint);
        if (ret == True) {
            break;
        }
    }

    return r;
}

Overlay::Overlay(bool automate, int interval, QWidget *parent) :
    QWidget(parent, Qt::BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::WindowStaysOnTopHint),
    board(N+2*PADDING, vector<int>(N+2*PADDING, -1)),
    display(XOpenDisplay(NULL)),
    root_win(XDefaultRootWindow(display))
{
    setAttribute(Qt::WA_TranslucentBackground);

    setWindowTitle("Terminal");

    resize(window()->screen()->size());

    QTimer *t = new QTimer(this); t->setTimerType(Qt::PreciseTimer);
    connect(t, SIGNAL(timeout()), this, SLOT(update()));
    t->start(100);

    if (automate) {
        t = new QTimer(this); t->setTimerType(Qt::PreciseTimer);
        connect(t, SIGNAL(timeout()), this, SLOT(make_move()));
        t->start(interval);
    }
}

Overlay::~Overlay() {
    XCloseDisplay(display);
}

void Overlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto game = window_from_name_search(display, root_win, (char *)WINDOW_NAME);
    if (!is_game_visible(game)) {
        return;
    }

    auto img = X11("").getImage(game, x0, y0, N*w, N*w);
    get_board(img, p);
    XDestroyImage(img);

#ifndef NDEBUG
    // draw a rectagle around the board (for debugging)
    p.setPen(QPen(Qt::blue, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF(x0, y0, N * w, N *w));
#endif

    // draw a line for each solution
    p.setPen(QPen(Qt::magenta, 7.0));
    auto solutions = calc_solutions();
    for(const auto& s : solutions) {
        const int x1 = x0 + (s.j1) * w + 0.5 * w, y1 = y0 + (s.i1) * w + 0.5 * w;
        const int x2 = x0 + (s.j2) * w + 0.5 * w, y2 = y0 + (s.i2) * w + 0.5 * w;
        p.drawLine(x1,y1,x2,y2);
    }
}

bool Overlay::is_game_visible(Window game) {
    if (game == 0) {
        return false;
    }

    XWindowAttributes attr;
    if (XGetWindowAttributes(display, game, &attr) == 0 ||
        attr.width != 1920 || attr.height != 1080 ||
        !is_window_visible(display, game)) {
        return false;
    }

    return true;
}

int Overlay::match_color(const vector<int>& c, vector<vector<int>>& palette, int threshold) {
    for (size_t k = 0; k < palette.size(); k++) {
        const auto p = palette[k];
        if (abs(c[0] - p[0]) < threshold &&
            abs(c[1] - p[1]) < threshold &&
            abs(c[2] - p[2]) < threshold) {
            return k;
        }
    }
    palette.push_back(c);
    return palette.size() - 1;
}

void Overlay::get_board(XImage* img, QPainter& p) {
    Q_UNUSED(p);
    vector<vector<int>> pallete;

    const float cell_w = 0.6 * w, margin = 0.5 * (w - cell_w), n_pixels = cell_w * cell_w;

    for (int m = 0; m < N; ++m ) {
        for (int n =0; n < N; ++n) {
            vector<int> rgb(3);
            const int x = n * w + margin, y = m * w + margin;
            for (int i = 0; i < cell_w; ++i) {
                for (int j = 0; j < cell_w; ++j) {
                    const QColor c(XGetPixel(img, x + i, y + j));
                    rgb[0] += c.red();
                    rgb[1] += c.green();
                    rgb[2] += c.blue();
                }
            }
            rgb[0] = rgb[0] / n_pixels;
            rgb[1] = rgb[1] / n_pixels;
            rgb[2] = rgb[2] / n_pixels;
            board[m+PADDING][n+PADDING] = match_color(rgb, pallete, 15);
#ifndef NDEBUG
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(QColor(rgb[0], rgb[1], rgb[2])));
            p.drawRect(QRect(x0+x-margin, y0+y-margin, margin, margin));
            p.setPen(Qt::red);
            p.setFont(QFont("Monospace", 8));
            p.drawText(x0+x, y0+y, QString::number(board[m+PADDING][n+PADDING]));
#endif
        }
    }
}

Solution Overlay::get_best_move() {
    auto solutions = calc_solutions();
    if (solutions.empty()) {
        return {-1, -1, -1, -1};
    }
    sort(solutions.begin(), solutions.end(), [](const auto& lhs, const auto& rhs){
        return lhs.i1 > rhs.i1 || lhs.i2 > rhs.i2;
    });

    return solutions[0];
}

void Overlay::make_move() {
    const auto game = window_from_name_search(display, root_win, (char *)WINDOW_NAME);
    if (!is_game_visible(game)) {
        return;
    }
    auto p = get_mouse_position(display);
    if (p.first < x0 || p.first > x0 + N * w || p.second < y0 || p.second > y0 + N * w) {
        return;
    }
    auto s = get_best_move();
    if (s.i1 == -1) {
        return;
    }
    const auto left_btn = 1; //XKeysymToKeycode(display, XK_Pointer_Button1);
    const auto screen = DefaultScreen(display);
    move_mouse(display, (s.j1 + 0.5) * w + x0, (s.i1 + 0.5) * w + y0, screen);
    mouse_click(display, left_btn);
    usleep(rand() % 5000 + 1000);
    move_mouse(display, (s.j2 + 0.5) * w + x0, (s.i2 + 0.5) * w + y0, screen);
    mouse_click(display, left_btn);
}

vector<Solution> Overlay::calc_solutions() {
    vector<Solution> s;
    for (int m = N-1+PADDING; m >= PADDING; --m ) {
        for (int n = PADDING; n < N+PADDING; ++n) {
            const int c = board[m][n], cr = board[m][n+1], ct = board[m-1][n];
            // swap with the right cell
            if (cr != -1) {
                if ((c == board[m][n+2] && c == board[m][n+3])
                 || (c == board[m-1][n+1] && c == board[m-2][n+1])
                 || (c == board[m+1][n+1] && c == board[m+2][n+1])
                 || (c == board[m-1][n+1] && c == board[m+1][n+1])
                 || (cr == board[m][n-1] && cr == board[m][n-2])
                 || (cr == board[m-1][n] && cr == board[m-2][n])
                 || (cr == board[m+1][n] && cr == board[m+2][n])
                 || (cr == board[m-1][n] && cr == board[m+1][n])
                ) {
                    s.push_back({m - PADDING, n - PADDING, m - PADDING, n + 1 - PADDING});
                }
            }
            // swap with the top cell
            if (ct != -1) {
                if ((c == board[m-2][n] && c == board[m-3][n])
                 || (c == board[m-1][n-1] && c == board[m-1][n-2])
                 || (c == board[m-1][n+1] && c == board[m-1][n+2])
                 || (c == board[m-1][n-1] && c == board[m-1][n+1])
                 || (ct == board[m+1][n] && ct == board[m+2][n])
                 || (ct == board[m][n-1] && ct == board[m][n-2])
                 || (ct == board[m][n+1] && ct == board[m][n+2])
                 || (ct == board[m][n-1] && ct == board[m][n+1])
                ) {
                    s.push_back({m - PADDING, n - PADDING, m - 1 - PADDING, n - PADDING});
                }
            }
        }
    }

    return s;
}
