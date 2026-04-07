#include "clipboard-history-store.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

#include <QDebug>

namespace {

const QString kStart = QStringLiteral("@CS$#");
const QString kEnd = QStringLiteral("@CE$#");
const QString kImageMarker = QStringLiteral("__IMAGE__");

QString makeImageHistoryBlock(const QString& hashHex, int w, int h)
{
    return kImageMarker + QLatin1Char('\n') + hashHex + QLatin1Char('\n')
        + QString::number(w) + QLatin1Char('x') + QString::number(h) + QLatin1Char('\n')
        + ClipboardHistoryStore::imageSearchTag();
}

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

QString ClipboardHistoryStore::imageSearchTag()
{
    return QStringLiteral("Imagem");
}

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
    ensureMediaDirs();
}

QString ClipboardHistoryStore::baseDataDir() const
{
    return QFileInfo(m_path).absoluteDir().absolutePath();
}

QString ClipboardHistoryStore::imagesDir() const
{
    return QFileInfo(m_path).absoluteDir().absoluteFilePath(QStringLiteral("images"));
}

QString ClipboardHistoryStore::thumbsDir() const
{
    return QFileInfo(m_path).absoluteDir().absoluteFilePath(QStringLiteral("thumbs"));
}

void ClipboardHistoryStore::ensureMediaDirs() const
{
    QDir().mkpath(imagesDir());
    QDir().mkpath(thumbsDir());
}

QString ClipboardHistoryStore::imageFilePath(const QString& hashHex) const
{
    return QDir(imagesDir()).filePath(hashHex + QStringLiteral(".png"));
}

QString ClipboardHistoryStore::thumbFilePath(const QString& hashHex) const
{
    return QDir(thumbsDir()).filePath(hashHex + QStringLiteral(".jpg"));
}

bool ClipboardHistoryStore::entryBlockIsImage(const QString& entryBlock)
{
    return entryBlock.startsWith(kImageMarker + QLatin1Char('\n'));
}

bool ClipboardHistoryStore::parseImageEntry(const QString& entryBlock,
                                            QString* outHashHex,
                                            QSize* outPixelSize)
{
    if (!entryBlockIsImage(entryBlock) || outHashHex == nullptr) {
        return false;
    }
    const QStringList lines = entryBlock.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    if (lines.size() < 2) {
        return false;
    }
    if (lines.at(0) != kImageMarker) {
        return false;
    }
    *outHashHex = lines.at(1);
    if (outHashHex->isEmpty()) {
        return false;
    }
    if (outPixelSize != nullptr && lines.size() >= 3) {
        const QString dim = lines.at(2);
        if (!dim.isEmpty()) {
            const int xPos = dim.indexOf(QLatin1Char('x'));
            if (xPos > 0 && xPos < dim.size() - 1) {
                bool okW = false;
                bool okH = false;
                const int w = dim.left(xPos).toInt(&okW);
                const int h = dim.mid(xPos + 1).toInt(&okH);
                if (okW && okH && w > 0 && h > 0) {
                    *outPixelSize = QSize(w, h);
                }
            }
        }
    }
    return true;
}

QString ClipboardHistoryStore::saveImageForHistory(const QImage& image)
{
    if (image.isNull()) {
        return {};
    }
    ensureMediaDirs();

    QImage img = image;
    if (img.format() == QImage::Format_Invalid) {
        return {};
    }

    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    if (!buffer.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (!img.save(&buffer, "PNG")) {
        return {};
    }

    const QByteArray hash =
        QCryptographicHash::hash(pngBytes, QCryptographicHash::Sha256).toHex();

    const QString hashHex = QString::fromLatin1(hash);
    const QString pngPath = imageFilePath(hashHex);
    if (!QFile::exists(pngPath)) {
        QSaveFile out(pngPath);
        if (!out.open(QIODevice::WriteOnly)) {
            return {};
        }
        out.write(pngBytes);
        if (!out.commit()) {
            return {};
        }
    }

    const QString jpgPath = thumbFilePath(hashHex);
    if (!QFile::exists(jpgPath)) {
        QImage thumb = img.scaled(kThumbMaxEdgePx,
                                  kThumbMaxEdgePx,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
        QImageWriter writer(jpgPath, QByteArrayLiteral("jpeg"));
        writer.setQuality(kThumbJpegQuality);
        if (!writer.write(thumb)) {
            qWarning() << "clipboard-monitor: failed to write thumbnail JPEG:" << jpgPath;
        }
    }

    return makeImageHistoryBlock(hashHex, img.width(), img.height());
}

void ClipboardHistoryStore::removeMediaForEntryIfImage(const QString& entryBlock)
{
    QString hash;
    if (!parseImageEntry(entryBlock, &hash, nullptr)) {
        return;
    }
    QFile::remove(imageFilePath(hash));
    QFile::remove(thumbFilePath(hash));
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
