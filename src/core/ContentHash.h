#pragma once

#include <QString>
#include <QCryptographicHash>

namespace CodeShelf {

class ContentHash {
public:
    // Calculates SHA-256 hash of the exact raw text (preserving all bytes and whitespace)
    static QString computeExactHash(const QString& content);

    // Calculates SHA-256 hash after normalizing line endings (\r\n -> \n) and trimming trailing whitespace on lines.
    // Strictly used for comparison heuristics; never alters stored content.
    static QString computeNormalizedHash(const QString& content);
};

} // namespace CodeShelf
