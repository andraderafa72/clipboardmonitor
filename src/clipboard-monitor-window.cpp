#include "clipboard-monitor-window.h"

#include <QAction>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>
#include <QShortcut>

ClipboardMonitor::ClipboardMonitor(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Clipboard Monitor"));

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowTitleHint);
    setProperty("_NET_WM_WINDOW_TYPE", QVariant(QStringLiteral("_NET_WM_WINDOW_TYPE_NORMAL")));
    setProperty("_NET_WM_STATE",
                QVariant::fromValue(QVariantList() << QStringLiteral("_NET_WM_STATE_ABOVE")));

    resize(400, 300);

    layout = new QVBoxLayout(this);
    center_window();

    listWidget = new QListWidget(this);
    layout->addWidget(listWidget);
    connect(listWidget, &QListWidget::itemClicked, this, &ClipboardMonitor::on_item_clicked);

    create_clear_button();

    clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardMonitor::on_clipboard_changed);

    auto* closeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(closeShortcut, &QShortcut::activated, this, &ClipboardMonitor::hide_window);

    trayIcon = new QSystemTrayIcon(this);
    QIcon trayIconImage(QStringLiteral(":/icons/clipboard-monitor.svg"));
    if (trayIconImage.isNull()) {
        qWarning("Ícone embutido ausente; bandeja pode fic sem ícone.");
    }
    trayIcon->setIcon(trayIconImage);
    trayIcon->show();

    auto* trayMenu = new QMenu(this);
    auto* showAction = new QAction(QStringLiteral("Mostrar Janela"), this);
    auto* exitAction = new QAction(QStringLiteral("Sair"), this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);

    connect(showAction, &QAction::triggered, this, &ClipboardMonitor::show_window);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    const QStringList loaded = m_history.loadNewestFirst();
    for (const QString& s : loaded) {
        listWidget->addItem(s);
    }
}

ClipboardMonitor::~ClipboardMonitor() = default;

void ClipboardMonitor::create_clear_button()
{
    auto* clearButton = new QPushButton(QStringLiteral("Limpar Lista"), this);
    layout->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, &ClipboardMonitor::clear_list);
}

void ClipboardMonitor::center_window()
{
    const QSize screenSize = QApplication::primaryScreen()->size();
    const int width = this->width();
    const int height = this->height();
    const int x = (screenSize.width() - width) / 2;
    const int y = (screenSize.height() - height) / 2;
    move(x, y);
}

void ClipboardMonitor::on_item_clicked(QListWidgetItem* item)
{
    const QString text = item->text();
    m_text_clicked = text;
    QApplication::clipboard()->setText(text);
    QMessageBox::information(this, QStringLiteral("Texto Copiado"),
                               QStringLiteral("Texto copiado para a área de transferência!"));
}

void ClipboardMonitor::show_window()
{
    show();
    center_window();
    raise();
    activateWindow();
}

void ClipboardMonitor::hide_window()
{
    hide();
}

QStringList ClipboardMonitor::listTextsNewestFirst() const
{
    QStringList out;
    const int n = listWidget->count();
    out.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (QListWidgetItem* it = listWidget->item(i)) {
            out.append(it->text());
        }
    }
    return out;
}

void ClipboardMonitor::trimListToMaxAndSyncFile()
{
    bool trimmed = false;
    while (listWidget->count() > ClipboardHistoryStore::kMaxEntries) {
        delete listWidget->takeItem(listWidget->count() - 1);
        trimmed = true;
    }
    if (trimmed) {
        m_history.rewriteFromNewestFirst(listTextsNewestFirst());
    }
}

void ClipboardMonitor::on_clipboard_changed()
{
    const QString text = clipboard->text();
    if (!text.isEmpty() && m_last_copied_text != text && m_text_clicked != text) {
        m_last_copied_text = text;
        listWidget->insertItem(0, text);
        m_history.append(text);
        trimListToMaxAndSyncFile();
    }
}

void ClipboardMonitor::clear_list()
{
    listWidget->clear();
    m_history.clear();
}

void ClipboardMonitor::closeEvent(QCloseEvent* event)
{
    event->ignore();
    hide();
}

void ClipboardMonitor::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        if (QApplication::activeModalWidget() != nullptr) {
            QWidget::changeEvent(event);
            return;
        }
        event->ignore();
        hide();
    } else {
        QWidget::changeEvent(event);
    }
}
