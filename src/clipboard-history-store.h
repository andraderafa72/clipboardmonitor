#ifndef CLIPBOARD_HISTORY_STORE_H
#define CLIPBOARD_HISTORY_STORE_H

#include <QString>
#include <QStringList>

class ClipboardHistoryStore {
public:
    static constexpr int kMaxEntries = 500;

    ClipboardHistoryStore();

    [[nodiscard]] QString path() const { return m_path; }

    [[nodiscard]] QStringList loadNewestFirst() const;
    void append(const QString& text);
    void clear();
    void rewriteFromNewestFirst(const QStringList& itemsNewestFirst);

private:
    QString m_path;

    static QString defaultHistoryPath();
};

#endif
