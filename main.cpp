#include <QApplication>
#include <QObject>
#include <QString>
#include <QLockFile> 
#include <QDir>
#include <string>
#include "src/keyboard-monitor.h"
#include "src/clipboard-monitor-window.h"

std::vector<std::pair<int, unsigned int>> get_keys_to_monitor(ClipboardMonitor &monitor){
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        throw std::string("Erro: Não foi possível abrir o display X11.");
    }

    int keycodeV = XKeysymToKeycode(display, XStringToKeysym("v"));
    unsigned int winModifier = Mod4Mask;
    XCloseDisplay(display);

    std::vector<std::pair<int, unsigned int>> keys = {
        {keycodeV, winModifier}
    };

    return keys;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Diretório para o arquivo de bloqueio
    QString lockFilePath = QDir::temp().absoluteFilePath("clipboard-monitor.lock");
    QLockFile lockFile(lockFilePath);

    // Configura o tempo limite para evitar bloqueios travados
    lockFile.setStaleLockTime(30000); // 30 segundos

    // Tenta obter o bloqueio
    if (!lockFile.tryLock()) {
        qDebug() << "Outra instância já está rodando.";
        return 1; // Sai se já existir uma instância
    }

    ClipboardMonitor monitor;
    KeyListener keyListener;

    try{
        std::vector<std::pair<int, unsigned int>>  keys = get_keys_to_monitor(monitor);

        if (!keyListener.start(keys)) {
            return 1;
        }

        QObject::connect(&keyListener, &KeyListener::keyPressed, [&monitor](int keycode) {
            monitor.show();
        });
    }
    catch (std::string error){
        qCritical(error.c_str());
        return 1;
    }

    return app.exec();
}
