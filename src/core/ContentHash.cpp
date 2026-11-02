#include "ContentHash.h"
#include <QStringList>

namespace CodeShelf {

QString ContentHash::computeExactHash(const QString& content) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(content.toUtf8());
    return QString::fromLatin1(hash.result().toHex());
}

QString ContentHash::computeNormalizedHash(const QString& content) {
    // Normalize line endings to \n
    QString normalized = content;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    // Split lines, trim trailing whitespace per line, ignore trailing empty lines
    const QStringList lines = normalized.split(QLatin1Char('\n'));
    QStringList cleanedLines;
    cleanedLines.reserve(lines.size());

    for (const QString& line : lines) {
        int i = line.size() - 1;
        while (i >= 0 && (line[i] == QLatin1Char(' ') || line[i] == QLatin1Char('\t'))) {
            --i;
        }
        cleanedLines.append(line.left(i + 1));
    }

    // Trim trailing empty lines
    while (!cleanedLines.isEmpty() && cleanedLines.last().isEmpty()) {
        cleanedLines.removeLast();
    }

    const QString joined = cleanedLines.join(QLatin1Char('\n'));
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(joined.toUtf8());
    return QString::fromLatin1(hash.result().toHex());
}

} // namespace CodeShelf
