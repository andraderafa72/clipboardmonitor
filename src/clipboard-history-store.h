#ifndef CLIPBOARD_HISTORY_STORE_H
#define CLIPBOARD_HISTORY_STORE_H

#include <QSize>
#include <QString>
#include <QStringList>

class QImage;

class ClipboardHistoryStore {
public:
    static constexpr int kMaxEntries = 500;
    static constexpr int kThumbMaxEdgePx = 128;
    static constexpr int kThumbJpegQuality = 70;

    /** Search/display label for image entries; single source for history block text and UI. */
    [[nodiscard]] static QString imageSearchTag();

    ClipboardHistoryStore();

    [[nodiscard]] QString path() const { return m_path; }
    [[nodiscard]] QString pinsPath() const { return m_pinsPath; }
    [[nodiscard]] QString baseDataDir() const;
    [[nodiscard]] QString imagesDir() const;
    [[nodiscard]] QString thumbsDir() const;

    void ensureMediaDirs() const;
    [[nodiscard]] QString saveImageForHistory(const QImage& image);
    [[nodiscard]] QString imageFilePath(const QString& hashHex) const;
    [[nodiscard]] QString thumbFilePath(const QString& hashHex) const;
    void removeMediaForEntryIfImage(const QString& entryBlock);

    [[nodiscard]] QStringList loadNewestFirst() const;
    [[nodiscard]] QStringList loadPinsNewestFirst() const;
    void rewriteFromNewestFirst(const QStringList& itemsNewestFirst);
    void rewritePinsNewestFirst(const QStringList& itemsNewestFirst);

    [[nodiscard]] static bool entryBlockIsImage(const QString& entryBlock);
    [[nodiscard]] static bool parseImageEntry(const QString& entryBlock,
                                              QString* outHashHex,
                                              QSize* outPixelSize = nullptr);

private:
    QString m_path;
    QString m_pinsPath;

    static QString defaultHistoryPath();
};

#endif
