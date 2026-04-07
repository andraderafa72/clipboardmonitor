#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QStandardPaths>

#include "clipboard-monitor-window.h"
#include "keyboard-monitor.h"

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

    if (!keyListener.startSuperVGrab()) {
        return 1;
    }

    QObject::connect(&keyListener, &KeyListener::superVHotkeyPressed, &monitor,
                     &ClipboardMonitor::show_window);

    return app.exec();
}
