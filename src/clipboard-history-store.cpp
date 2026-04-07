#include "clipboard-history-store.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

namespace {

const QString kStart = QStringLiteral("@CS$#");
const QString kEnd = QStringLiteral("@CE$#");

QStringList loadBlocksNewestFirstFromPath(const QString& filePath)
{
    QStringList chronological;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
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
            chronological.append(block);
        }
    }

    std::reverse(chronological.begin(), chronological.end());
    while (chronological.size() > ClipboardHistoryStore::kMaxEntries) {
        chronological.removeLast();
    }
    return chronological;
}

void rewriteNewestFirstToPath(const QString& filePath,
                              const QStringList& itemsNewestFirst)
{
    QFile file(filePath);
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
    , m_pinsPath(
          QFileInfo(m_path).absoluteDir().absoluteFilePath(QStringLiteral("pins.txt")))
{
}

QStringList ClipboardHistoryStore::loadNewestFirst() const
{
    return loadBlocksNewestFirstFromPath(m_path);
}

QStringList ClipboardHistoryStore::loadPinsNewestFirst() const
{
    return loadBlocksNewestFirstFromPath(m_pinsPath);
}

void ClipboardHistoryStore::rewriteFromNewestFirst(const QStringList& itemsNewestFirst)
{
    rewriteNewestFirstToPath(m_path, itemsNewestFirst);
}

void ClipboardHistoryStore::rewritePinsNewestFirst(const QStringList& itemsNewestFirst)
{
    rewriteNewestFirstToPath(m_pinsPath, itemsNewestFirst);
}
