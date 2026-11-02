#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace CodeShelf {

enum class DiffLineType {
    Context,   // Unchanged ' '
    Addition,  // Added in right '+'
    Deletion   // Removed in left '-'
};

struct DiffLine {
    DiffLineType type{DiffLineType::Context};
    int oldLineNumber{0}; // 1-based, 0 if not applicable
    int newLineNumber{0}; // 1-based, 0 if not applicable
    QString text;
    QString prefixMarker() const {
        switch (type) {
            case DiffLineType::Context: return QStringLiteral("  ");
            case DiffLineType::Addition: return QStringLiteral("+ ");
            case DiffLineType::Deletion: return QStringLiteral("- ");
        }
        return QStringLiteral("  ");
    }
};

class DiffGenerator {
public:
    static QVector<DiffLine> generateDiff(const QString& oldText, const QString& newText, bool ignoreLineEndings = true);
    static QString formatUnifiedDiff(const QVector<DiffLine>& diffLines);
};

} // namespace CodeShelf
