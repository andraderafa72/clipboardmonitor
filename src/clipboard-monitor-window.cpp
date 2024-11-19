#include "clipboard-monitor-window.h"
#include <iostream>

ClipboardMonitor::~ClipboardMonitor() {
    save_clipboard_history();
}

ClipboardMonitor::ClipboardMonitor(QWidget *parent){
    // Configuração da janela
    std::string user = std::getenv("USER");
    std::string path = "/home/" + std::string(user) + "/.clipboard_history";
    m_filepath = QString(path.c_str());
    
    setWindowTitle("Clipboard Monitor");
    resize(400, 300);

    // Layout
    layout = new QVBoxLayout(this);
    center_window();
    
    // Lista para exibir os textos copiados
    listWidget = new QListWidget(this);
    layout->addWidget(listWidget);  
    connect(listWidget, &QListWidget::itemClicked, this, &ClipboardMonitor::on_item_clicked);

    create_clear_button();

    // Iniciar o monitoramento do clipboard
    clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardMonitor::on_clipboard_changed);

    // Carregar histórico de clipboard
    load_clipboard_history();
}

void ClipboardMonitor::create_clear_button(){
    QPushButton *clearButton = new QPushButton("Limpar Lista", this);
    layout->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, &ClipboardMonitor::clear_list);
}

void ClipboardMonitor::center_window(){
    QSize screenSize = QApplication::primaryScreen()->size();
    int width = this->width();
    int height = this->height();
    int x = (screenSize.width() - width) / 2;
    int y = (screenSize.height() - height) / 2;
    move(x, y);
}

void ClipboardMonitor::on_item_clicked(QListWidgetItem *item) {
    QString text = item->text();
    m_text_clicked = text;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);

    QMessageBox::information(this, "Texto Copiado", "Texto copiado para a área de transferência!");
}

void ClipboardMonitor::on_clipboard_changed() {
    QString text = clipboard->text();
    if (!text.isEmpty() && m_text_clicked != text) {
        listWidget->addItem(text);
        // Salvar no histórico
        save_clipboard_history(text);
    }
}

void ClipboardMonitor::save_clipboard_history(const QString &newText) {
    QFile file(m_filepath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        if (!newText.isEmpty()) {
            out << CLIPBOARD_START_DELIMITOR << "\n" 
            << newText << "\n"
            << CLIPBOARD_END_DELIMITOR << "\n";
        }
    }
}

void ClipboardMonitor::load_clipboard_history() {
    QFile file(m_filepath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QString result;

            if(line == CLIPBOARD_START_DELIMITOR){
                bool firstLine = true;
                while(line != CLIPBOARD_END_DELIMITOR){
                    line = in.readLine();
                    if(line == CLIPBOARD_END_DELIMITOR) break;
                    
                    if(!firstLine){
                        result.append("\n");
                    } else {
                        firstLine = false;
                    }

                    result.append(line);
                }
            }

            listWidget->addItem(result);
        }
    }
}

void ClipboardMonitor::clear_clipboard_history() {
    std::ofstream file(m_filepath.toStdString(), std::ios::trunc);
    file.close();
}

void ClipboardMonitor::clear_list(){
    listWidget->clear();
    clear_clipboard_history();
}