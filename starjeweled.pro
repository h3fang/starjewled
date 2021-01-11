QT += core widgets
CONFIG += c++17 warn_on

TARGET = terminal

release {
    DEFINES += NDEBUG
}

HEADERS           = src/overlay.h \
                    src/x.hpp \
                    src/mouse.h

SOURCES           = src/main.cpp \
                    src/overlay.cpp \
                    src/x.cpp \
                    src/mouse.cpp

LIBS += -lX11 -lXext -lXrandr -lXrender -lXcomposite -lXfixes -lXtst
