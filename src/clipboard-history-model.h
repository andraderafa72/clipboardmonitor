#ifndef CLIPBOARD_HISTORY_MODEL_H
#define CLIPBOARD_HISTORY_MODEL_H

#include "clipboard-history-store.h"

#include <QPair>
#include <QStringList>
#include <QVector>

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
    void clearHistory();

private:
    void trimHistory();
    void trimPins();
    void dedupeHistoryAgainstPins();

    QStringList m_pins;
    QStringList m_hist;
};

#endif
