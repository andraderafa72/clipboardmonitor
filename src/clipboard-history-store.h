#ifndef CLIPBOARD_HISTORY_STORE_H
#define CLIPBOARD_HISTORY_STORE_H

#include <QString>
#include <QStringList>

class ClipboardHistoryStore {
public:
    static constexpr int kMaxEntries = 500;

    ClipboardHistoryStore();

    [[nodiscard]] QString path() const { return m_path; }
    [[nodiscard]] QString pinsPath() const { return m_pinsPath; }

    [[nodiscard]] QStringList loadNewestFirst() const;
    [[nodiscard]] QStringList loadPinsNewestFirst() const;
    void rewriteFromNewestFirst(const QStringList& itemsNewestFirst);
    void rewritePinsNewestFirst(const QStringList& itemsNewestFirst);

private:
    QString m_path;
    QString m_pinsPath;

    static QString defaultHistoryPath();
};

#endif
