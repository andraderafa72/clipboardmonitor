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
#include <string>
#include <fstream>

const QString CLIPBOARD_START_DELIMITOR = "@CS$#";
const QString CLIPBOARD_END_DELIMITOR = "@CE$#";

class ClipboardMonitor : public QWidget {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QWidget *parent = nullptr);
    virtual ~ClipboardMonitor();

private slots:
    void on_item_clicked(QListWidgetItem *item);
    void on_clipboard_changed();

    void clear_list();

private:
    QString m_filepath;
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
};

