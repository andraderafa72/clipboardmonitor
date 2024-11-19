#include <QApplication>
#include "src/clipboard-monitor-window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ClipboardMonitor monitor;
    monitor.show();

    return app.exec();
}
