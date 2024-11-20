#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include <QShortcut>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QScreen>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QEvent>
#include <QHideEvent>
#include <string>
#include <fstream>

const QString CLIPBOARD_START_DELIMITOR = "@CS$#";
const QString CLIPBOARD_END_DELIMITOR = "@CE$#";

class ClipboardMonitor : public QWidget {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QWidget *parent = nullptr);
    virtual ~ClipboardMonitor();

    void show_window();
    void hide_window();
private slots:
    void on_item_clicked(QListWidgetItem *item);
    void on_clipboard_changed();

    void clear_list();

private:
    QString m_filepath;
    QString m_icon_path = QApplication::applicationDirPath() + "/icon.svg";
    QString m_text_clicked;

    void save_clipboard_history(const QString &newText = QString());
    void load_clipboard_history();
    void clear_clipboard_history();

    void center_window();
    void create_clear_button();
    
private:
    QClipboard *clipboard;
    QVBoxLayout *layout;
    QListWidget *listWidget;
    QSystemTrayIcon *trayIcon;

protected:
    void closeEvent(QCloseEvent *event) override {
        event->ignore();
        this->hide();
    }
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
            event->ignore();
            this->hide();
        } else {
            QWidget::changeEvent(event);
        }
    }
};

#endif