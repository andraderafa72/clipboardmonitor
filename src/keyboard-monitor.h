#ifndef KEYLISTENER_H
#define KEYLISTENER_H

#include <QObject>

#include <atomic>
#include <thread>
#include <utility>
#include <vector>

typedef struct _XDisplay Display;

class KeyListener : public QObject {
    Q_OBJECT

public:
    explicit KeyListener(QObject* parent = nullptr);
    ~KeyListener() override;

    bool start(const std::vector<std::pair<int, unsigned int>>& keys);
    void stop();

signals:
    void keyPressed(int keycode);

private:
    void grabKeys();
    void ungrabKeys();
    void eventLoop();

    Display* display;
    unsigned long rootWindow;
    std::thread listenerThread;
    std::atomic<bool> running{false};
    std::vector<std::pair<int, unsigned int>> keysToMonitor;
};

std::vector<std::pair<int, unsigned int>> superVHotkeys();

#endif // KEYLISTENER_H
