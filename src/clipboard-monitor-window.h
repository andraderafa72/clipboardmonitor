#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include "clipboard-history-model.h"
#include "clipboard-history-store.h"

#include <QShortcut>
#include <QApplication>
#include <QClipboard>
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
    void on_clipboard_changed();
    void clear_list();
    void on_list_context_menu(const QPoint& pos);

private:
    static constexpr int kRolePinned = Qt::UserRole;
    static constexpr int kRoleClipText = Qt::UserRole + 1;

    void reloadModelFromDisk();
    void persistModelToDisk();
    void rebuildListWidget();

    void togglePinForText(const QString& text);
    void removeText(const QString& text);

    [[nodiscard]] QString clipText(const QListWidgetItem* item) const;

    void copyItemTextToClipboard(QListWidgetItem* item);
    [[nodiscard]] QListWidgetItem* createListItem(const QString& text, bool pinned);
    void attachItemRowWidget(QListWidgetItem* item);
    void updateRowTextElision(QListWidgetItem* item);
    void refreshAllRowElisions();

    void center_window();
    void create_clear_button();

    ClipboardHistoryStore m_store;
    ClipboardHistoryModel m_model;

    QString m_text_clicked;
    QString m_last_copied_text;

    QClipboard* clipboard = nullptr;
    QVBoxLayout* layout = nullptr;
    QListWidget* listWidget = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
};

#endif
