#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include "clipboard-history-store.h"

#include <QShortcut>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>
#include <QEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QString>

class ClipboardMonitor : public QWidget {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QWidget* parent = nullptr);
    ~ClipboardMonitor() override;

    void show_window();
    void hide_window();

private slots:
    void on_item_clicked(QListWidgetItem* item);
    void on_clipboard_changed();
    void clear_list();

private:
    void trimListToMaxAndSyncFile();
    QStringList listTextsNewestFirst() const;

    void center_window();
    void create_clear_button();

    ClipboardHistoryStore m_history;
    QString m_text_clicked;
    QString m_last_copied_text;

    QClipboard* clipboard = nullptr;
    QVBoxLayout* layout = nullptr;
    QListWidget* listWidget = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
};

#endif
