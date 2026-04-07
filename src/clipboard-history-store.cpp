#include "clipboard-history-store.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

namespace {

const QString kStart = QStringLiteral("@CS$#");
const QString kEnd = QStringLiteral("@CE$#");

} // namespace

QString ClipboardHistoryStore::defaultHistoryPath()
{
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString dir = base + QStringLiteral("/clipboardmonitor");
    QDir().mkpath(dir);
    return dir + QStringLiteral("/history.txt");
}

ClipboardHistoryStore::ClipboardHistoryStore()
    : m_path(defaultHistoryPath())
{
}

QStringList ClipboardHistoryStore::loadNewestFirst() const
{
    QStringList out;
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return out;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line != kStart) {
            continue;
        }
        QString block;
        bool firstLine = true;
        while (!in.atEnd()) {
            line = in.readLine();
            if (line == kEnd) {
                break;
            }
            if (!firstLine) {
                block.append(QLatin1Char('\n'));
            } else {
                firstLine = false;
            }
            block.append(line);
        }
        if (!block.isEmpty()) {
            out.prepend(block);
        }
    }

    while (out.size() > kMaxEntries) {
        out.removeLast();
    }
    return out;
}

void ClipboardHistoryStore::append(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }
    QFile file(m_path);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << kStart << QLatin1Char('\n') << text << QLatin1Char('\n') << kEnd << QLatin1Char('\n');
}

void ClipboardHistoryStore::clear()
{
    QFile file(m_path);
    if (file.exists()) {
        file.remove();
    }
}

void ClipboardHistoryStore::rewriteFromNewestFirst(const QStringList& itemsNewestFirst)
{
    QFile file(m_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    const int n = itemsNewestFirst.size();
    for (int i = n - 1; i >= 0; --i) {
        const QString& t = itemsNewestFirst.at(i);
        if (t.isEmpty()) {
            continue;
        }
        out << kStart << QLatin1Char('\n') << t << QLatin1Char('\n') << kEnd << QLatin1Char('\n');
    }
}
