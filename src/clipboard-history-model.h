#ifndef CLIPBOARD_HISTORY_MODEL_H
#define CLIPBOARD_HISTORY_MODEL_H

#include "clipboard-history-store.h"

#include <QPair>
#include <QStringList>
#include <QVector>

/** History entries are QString blocks: plain text, or image refs from ClipboardHistoryStore::saveImageForHistory. */
class ClipboardHistoryModel {
public:
    static constexpr int kMaxPinnedEntries = 100;

    void load(ClipboardHistoryStore& store);
    void persist(ClipboardHistoryStore& store) const;

    [[nodiscard]] QVector<QPair<QString, bool>> buildRenderVector() const;

    enum class ClipboardApplyResult { NoOp, Updated };
    ClipboardApplyResult applyNewClipboardText(const QString& text,
                                               const QString& lastCopied,
                                               const QString& textClicked,
                                               QString& outLastCopied);

    void togglePin(const QString& text);
    void removeEntry(const QString& text);
    /** Removes non-pinned history entries and deletes on-disk image files for image blocks. */
    void clearHistory(ClipboardHistoryStore& store);

private:
    void trimHistory();
    void trimPins();
    void dedupeHistoryAgainstPins();

    QStringList m_pins;
    QStringList m_hist;
};

#endif
