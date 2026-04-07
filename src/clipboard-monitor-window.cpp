#include "clipboard-monitor-window.h"

#include <QAction>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QScreen>
#include <QShortcut>
#include <QSize>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

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
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidget->viewport()->installEventFilter(this);
    layout->addWidget(listWidget);
    connect(listWidget, &QListWidget::customContextMenuRequested, this,
            &ClipboardMonitor::on_list_context_menu);

    create_clear_button();

    clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardMonitor::on_clipboard_changed);

    auto* closeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(closeShortcut, &QShortcut::activated, this, &ClipboardMonitor::hide_window);

    trayIcon = new QSystemTrayIcon(this);
    QIcon trayIconImage(QStringLiteral(":/icons/clipboard-monitor.svg"));
    if (trayIconImage.isNull()) {
        qWarning("Ícone embutido ausente; bandeja pode ficar sem ícone.");
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

    reloadModelFromDisk();
    rebuildListWidget();
}

ClipboardMonitor::~ClipboardMonitor() = default;

void ClipboardMonitor::reloadModelFromDisk()
{
    m_model.load(m_store);
}

void ClipboardMonitor::persistModelToDisk()
{
    m_model.persist(m_store);
}

void ClipboardMonitor::rebuildListWidget()
{
    listWidget->clear();
    const QVector<QPair<QString, bool>> rows = m_model.buildRenderVector();
    for (const auto& pr : rows) {
        QListWidgetItem* it = createListItem(pr.first, pr.second);
        listWidget->addItem(it);
        attachItemRowWidget(it);
    }
}

void ClipboardMonitor::togglePinForText(const QString& text)
{
    m_model.togglePin(text);
    persistModelToDisk();
    rebuildListWidget();
}

void ClipboardMonitor::removeText(const QString& text)
{
    m_model.removeEntry(text);
    persistModelToDisk();
    rebuildListWidget();
}

bool ClipboardMonitor::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == listWidget->viewport() && event->type() == QEvent::Resize) {
        refreshAllRowElisions();
    }
    return QWidget::eventFilter(watched, event);
}

QString ClipboardMonitor::clipText(const QListWidgetItem* item) const
{
    if (item == nullptr) {
        return {};
    }
    const QVariant v = item->data(kRoleClipText);
    if (v.isValid()) {
        return v.toString();
    }
    return item->text();
}

void ClipboardMonitor::copyItemTextToClipboard(QListWidgetItem* item)
{
    const QString t = clipText(item);
    m_text_clicked = t;
    QApplication::clipboard()->setText(t);
    if (trayIcon != nullptr && trayIcon->isVisible()) {
        trayIcon->showMessage(QStringLiteral("Clipboard Monitor"),
                              QStringLiteral("Texto copiado para a área de transferência."),
                              QSystemTrayIcon::MessageIcon::Information, 2500);
    }
}

QListWidgetItem* ClipboardMonitor::createListItem(const QString& text, bool pinned)
{
    auto* item = new QListWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setData(kRolePinned, pinned);
    item->setData(kRoleClipText, text);
    item->setText(QString());
    item->setSizeHint(QSize(0, 40));
    return item;
}

void ClipboardMonitor::attachItemRowWidget(QListWidgetItem* item)
{
    const QString rowText = item->data(kRoleClipText).toString();

    auto* row = new QWidget(listWidget);
    auto* h = new QHBoxLayout(row);
    h->setContentsMargins(4, 2, 4, 2);
    h->setSpacing(6);

    auto* textBtn = new QPushButton(row);
    textBtn->setObjectName(QStringLiteral("clipboardTextButton"));
    textBtn->setFlat(true);
    textBtn->setCursor(Qt::PointingHandCursor);
    textBtn->setFocusPolicy(Qt::NoFocus);
    textBtn->setStyleSheet(QStringLiteral("QPushButton { text-align: left; padding: 2px 4px; }"));

    auto* pinBtn = new QToolButton(row);
    pinBtn->setObjectName(QStringLiteral("pinToolButton"));
    pinBtn->setCheckable(true);
    pinBtn->setAutoRaise(true);
    pinBtn->setFocusPolicy(Qt::NoFocus);
    pinBtn->setFixedSize(28, 28);
    pinBtn->setText(QStringLiteral("📌"));

    const bool p = item->data(kRolePinned).toBool();
    pinBtn->setChecked(p);
    pinBtn->setToolTip(p ? QStringLiteral("Desafixar (remove do topo)")
                         : QStringLiteral("Fixar no topo da lista"));

    h->addWidget(textBtn, 1);
    h->addWidget(pinBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    connect(textBtn, &QPushButton::clicked, this, [this, item]() {
        copyItemTextToClipboard(item);
    });

    connect(pinBtn, &QToolButton::clicked, this, [this, rowText]() {
        QTimer::singleShot(0, this, [this, rowText]() {
            togglePinForText(rowText);
        });
    });

    listWidget->setItemWidget(item, row);
    updateRowTextElision(item);
}

void ClipboardMonitor::updateRowTextElision(QListWidgetItem* item)
{
    QWidget* row = listWidget->itemWidget(item);
    if (row == nullptr) {
        return;
    }
    auto* textBtn =
        row->findChild<QPushButton*>(QStringLiteral("clipboardTextButton"), Qt::FindDirectChildrenOnly);
    if (textBtn == nullptr) {
        textBtn = row->findChild<QPushButton*>(QStringLiteral("clipboardTextButton"));
    }
    if (textBtn == nullptr) {
        return;
    }
    const QString full = clipText(item);
    constexpr int kPinReserve = 40;
    int vw = listWidget->viewport()->width();
    if (vw <= 1) {
        vw = listWidget->width();
    }
    const int w = qMax(64, vw - kPinReserve);
    const QFontMetrics fm(textBtn->font());
    QString elided = fm.elidedText(full, Qt::ElideRight, w);
    if (!full.isEmpty() && elided.isEmpty()) {
        elided = fm.elidedText(full, Qt::ElideRight, qMax(w, 200));
    }
    if (!full.isEmpty() && elided.isEmpty()) {
        elided = full.left(48) + QStringLiteral("…");
    }
    textBtn->setText(elided);
    textBtn->setToolTip(full);
}

void ClipboardMonitor::refreshAllRowElisions()
{
    const int n = listWidget->count();
    for (int i = 0; i < n; ++i) {
        if (QListWidgetItem* it = listWidget->item(i)) {
            updateRowTextElision(it);
        }
    }
}

void ClipboardMonitor::create_clear_button()
{
    auto* clearButton = new QPushButton(QStringLiteral("Limpar histórico"), this);
    clearButton->setToolTip(
        QStringLiteral("Remove entradas não fixadas do histórico. Pins permanecem."));
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

void ClipboardMonitor::on_list_context_menu(const QPoint& pos)
{
    QListWidgetItem* item = listWidget->itemAt(pos);
    if (item == nullptr) {
        return;
    }

    const QString entryText = clipText(item);
    const bool itemPinned = item->data(kRolePinned).toBool();

    QMenu menu(this);
    QAction* removeAction = menu.addAction(QStringLiteral("Remover"));
    QAction* pinAction =
        menu.addAction(itemPinned ? QStringLiteral("Desafixar") : QStringLiteral("Fixar"));

    QAction* chosen = menu.exec(listWidget->mapToGlobal(pos));
    if (chosen == nullptr) {
        return;
    }

    if (chosen == removeAction) {
        removeText(entryText);
        return;
    }
    if (chosen == pinAction) {
        togglePinForText(entryText);
    }
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

void ClipboardMonitor::on_clipboard_changed()
{
    const QString text = clipboard->text();
    QString newLast = m_last_copied_text;
    const ClipboardHistoryModel::ClipboardApplyResult r =
        m_model.applyNewClipboardText(text, m_last_copied_text, m_text_clicked, newLast);
    if (r == ClipboardHistoryModel::ClipboardApplyResult::NoOp) {
        return;
    }
    m_last_copied_text = newLast;
    persistModelToDisk();
    rebuildListWidget();
}

void ClipboardMonitor::clear_list()
{
    m_model.clearHistory();
    persistModelToDisk();
    rebuildListWidget();
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
