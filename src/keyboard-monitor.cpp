#include "keyboard-monitor.h"

#include <poll.h>
#include <stdexcept>
#include <utility>

#define Bool int
#define None 0L
#include <X11/Xlib.h>
#include <X11/keysym.h>
#undef Bool
#undef None

namespace {

int xConnectionFd(Display* display)
{
    return ConnectionNumber(display);
}

} // namespace

KeyListener::KeyListener(QObject* parent)
    : QObject(parent)
    , display(nullptr)
    , rootWindow(0)
{
}

KeyListener::~KeyListener()
{
    stop();
}

void KeyListener::grabKeys()
{
    for (const auto& [keycode, modifier] : keysToMonitor) {
        if (keycode == 0) {
            continue;
        }
        XGrabKey(display, keycode, modifier, static_cast<Window>(rootWindow), True, GrabModeAsync,
                 GrabModeAsync);
    }
}

void KeyListener::ungrabKeys()
{
    if (!display) {
        return;
    }
    for (const auto& [keycode, modifier] : keysToMonitor) {
        if (keycode == 0) {
            continue;
        }
        XUngrabKey(display, keycode, modifier, static_cast<Window>(rootWindow));
    }
}

void KeyListener::eventLoop()
{
    const int fd = xConnectionFd(display);
    while (running.load(std::memory_order_acquire)) {
        pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLIN;
        const int pr = poll(&pfd, 1, 200);
        if (pr < 0) {
            continue;
        }
        if (!running.load(std::memory_order_acquire)) {
            break;
        }
        if (pr == 0) {
            continue;
        }
        while (running.load(std::memory_order_acquire) && XPending(display) > 0) {
            XEvent event{};
            XNextEvent(display, &event);
            if (event.type == KeyPress) {
                const int keycode = static_cast<int>(event.xkey.keycode);
                for (const auto& grabbed : keysToMonitor) {
                    if (grabbed.first != 0 && keycode == grabbed.first) {
                        emit superVHotkeyPressed();
                        break;
                    }
                }
            }
        }
    }
}

bool KeyListener::start(const std::vector<std::pair<int, unsigned int>>& keys)
{
    if (display != nullptr) {
        return false;
    }

    display = XOpenDisplay(nullptr);
    if (!display) {
        qCritical("Erro: Não foi possível abrir o display X11.");
        return false;
    }

    rootWindow = static_cast<unsigned long>(DefaultRootWindow(display));
    keysToMonitor = keys;
    grabKeys();
    XFlush(display);

    running.store(true, std::memory_order_release);
    listenerThread = std::thread([this]() { eventLoop(); });
    return true;
}

void KeyListener::stop()
{
    running.store(false, std::memory_order_release);
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
    if (display) {
        ungrabKeys();
        XCloseDisplay(display);
        display = nullptr;
        rootWindow = 0;
    }
}

bool KeyListener::startSuperVGrab()
{
    if (display != nullptr) {
        return false;
    }

    display = XOpenDisplay(nullptr);
    if (!display) {
        qCritical("Erro: Não foi possível abrir o display X11.");
        return false;
    }

    const KeyCode keycodeV = XKeysymToKeycode(display, XStringToKeysym("v"));
    if (keycodeV == 0) {
        qCritical("Erro: não foi possível obter keycode da tecla V.");
        XCloseDisplay(display);
        display = nullptr;
        return false;
    }

    rootWindow = static_cast<unsigned long>(DefaultRootWindow(display));
    keysToMonitor = {{static_cast<int>(keycodeV), Mod4Mask}};
    grabKeys();
    XFlush(display);

    running.store(true, std::memory_order_release);
    listenerThread = std::thread([this]() { eventLoop(); });
    return true;
}
