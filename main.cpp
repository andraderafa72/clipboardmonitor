#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QStandardPaths>

#include "clipboard-monitor-window.h"
#include "keyboard-monitor.h"

#include <exception>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("clipboardmonitor"));

    QString lockDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (lockDir.isEmpty()) {
        lockDir = QDir::tempPath();
    }
    QDir().mkpath(lockDir);
    const QString lockFilePath =
        QDir(lockDir).absoluteFilePath(QStringLiteral("clipboardmonitor.lock"));
    QLockFile lockFile(lockFilePath);
    lockFile.setStaleLockTime(30000);

    if (!lockFile.tryLock()) {
        qDebug() << "Outra instância já está rodando.";
        return 1;
    }

    ClipboardMonitor monitor;
    KeyListener keyListener;

    try {
        const std::vector<std::pair<int, unsigned int>> keys = superVHotkeys();
        if (!keyListener.start(keys)) {
            return 1;
        }

        QObject::connect(&keyListener, &KeyListener::keyPressed, &monitor, [&monitor](int /*keycode*/) {
            monitor.show_window();
        });
    } catch (const std::exception& ex) {
        qCritical("%s", ex.what());
        return 1;
    }

    return app.exec();
}
