#ifndef KEYLISTENER_H
#define KEYLISTENER_H

#include <QObject>

#define Bool int
#define None 0L

#include <X11/Xlib.h>
#include <X11/keysym.h>

#undef Bool
#undef None

#include <thread>
#include <functional>
#include <vector>

class KeyListener : public QObject {
    Q_OBJECT

private:
    Display* display;
    Window rootWindow;
    std::thread listenerThread;
    bool running;
    std::vector<std::pair<int, unsigned int>> keysToMonitor;

    void grabKeys() {
        for (const auto& [keycode, modifier] : keysToMonitor) {
            XGrabKey(display, keycode, modifier, rootWindow, True,
                     GrabModeAsync, GrabModeAsync);
        }
    }

    void ungrabKeys() {
        for (const auto& [keycode, modifier] : keysToMonitor) {
            XUngrabKey(display, keycode, modifier, rootWindow);
        }
    }

    void eventLoop() {
        XEvent event;
        while (running) {
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
                int keycode = event.xkey.keycode;
                emit keyPressed(keycode);
            }
        }
    }

public:
    explicit KeyListener(QObject* parent = nullptr) : QObject(parent), display(nullptr), rootWindow(0), running(false) {}

    ~KeyListener() {
        stop();
    }

    bool start(const std::vector<std::pair<int, unsigned int>>& keys) {
        display = XOpenDisplay(nullptr);
        if (!display) {
            qCritical("Erro: Não foi possível abrir o display X11.");
            return false;
        }

        rootWindow = DefaultRootWindow(display);
        running = true;

        keysToMonitor = keys;
        grabKeys();

        listenerThread = std::thread([this]() {
            eventLoop();
        });

        return true;
    }

    void stop() {
        if (running) {
            running = false;
            ungrabKeys();
            if (listenerThread.joinable()) {
                listenerThread.join();
            }
            if (display) {
                XCloseDisplay(display);
                display = nullptr;
            }
        }
    }

signals:
    void keyPressed(int keycode);
};

#endif // KEYLISTENER_H
