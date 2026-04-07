#include "clipboard-history-model.h"

#include <QSet>

void ClipboardHistoryModel::load(ClipboardHistoryStore& store)
{
    m_pins = store.loadPinsNewestFirst();
    m_hist = store.loadNewestFirst();
    dedupeHistoryAgainstPins();
    trimHistory();
    trimPins();
}

void ClipboardHistoryModel::persist(ClipboardHistoryStore& store) const
{
    store.rewritePinsNewestFirst(m_pins);
    store.rewriteFromNewestFirst(m_hist);
}

QVector<QPair<QString, bool>> ClipboardHistoryModel::buildRenderVector() const
{
    QVector<QPair<QString, bool>> out;
    QSet<QString> seen;
    const int reserve = m_pins.size() + m_hist.size();
    out.reserve(reserve);
    seen.reserve(reserve * 2 + 16);

    for (const QString& t : m_pins) {
        if (t.isEmpty() || seen.contains(t)) {
            continue;
        }
        seen.insert(t);
        out.append(qMakePair(t, true));
    }
    for (const QString& t : m_hist) {
        if (t.isEmpty() || seen.contains(t)) {
            continue;
        }
        seen.insert(t);
        out.append(qMakePair(t, false));
    }
    return out;
}

ClipboardHistoryModel::ClipboardApplyResult ClipboardHistoryModel::applyNewClipboardText(
    const QString& text,
    const QString& lastCopied,
    const QString& textClicked,
    QString& outLastCopied)
{
    if (text.isEmpty() || lastCopied == text || textClicked == text) {
        return ClipboardApplyResult::NoOp;
    }

    const int pi = m_pins.indexOf(text);
    if (pi >= 0) {
        m_pins.removeAt(pi);
        m_pins.prepend(text);
        outLastCopied = text;
        trimPins();
        return ClipboardApplyResult::Updated;
    }

    const int hi = m_hist.indexOf(text);
    if (hi >= 0) {
        m_hist.removeAt(hi);
        m_hist.prepend(text);
        outLastCopied = text;
        trimHistory();
        return ClipboardApplyResult::Updated;
    }

    m_hist.prepend(text);
    outLastCopied = text;
    trimHistory();
    return ClipboardApplyResult::Updated;
}

void ClipboardHistoryModel::togglePin(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    const int pi = m_pins.indexOf(text);
    if (pi >= 0) {
        m_pins.removeAt(pi);
        m_hist.removeAll(text);
        m_hist.prepend(text);
    } else {
        const int hi = m_hist.indexOf(text);
        if (hi >= 0) {
            m_hist.removeAt(hi);
        }
        m_pins.removeAll(text);
        m_pins.prepend(text);
    }
    trimHistory();
    trimPins();
}

void ClipboardHistoryModel::removeEntry(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }
    m_pins.removeAll(text);
    m_hist.removeAll(text);
    trimHistory();
    trimPins();
}

void ClipboardHistoryModel::clearHistory()
{
    m_hist.clear();
}

void ClipboardHistoryModel::trimHistory()
{
    while (m_hist.size() > ClipboardHistoryStore::kMaxEntries) {
        m_hist.removeLast();
    }
}

void ClipboardHistoryModel::trimPins()
{
    while (m_pins.size() > kMaxPinnedEntries) {
        m_pins.removeLast();
    }
}

void ClipboardHistoryModel::dedupeHistoryAgainstPins()
{
    const QSet<QString> pinSet(m_pins.begin(), m_pins.end());
    QStringList filtered;
    filtered.reserve(m_hist.size());
    for (const QString& h : m_hist) {
        if (!pinSet.contains(h)) {
            filtered.append(h);
        }
    }
    m_hist = std::move(filtered);
}
