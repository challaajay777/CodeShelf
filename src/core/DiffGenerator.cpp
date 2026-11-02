#include "DiffGenerator.h"
#include <vector>
#include <algorithm>

namespace CodeShelf {

namespace {

QStringList splitLines(const QString& text, bool ignoreLineEndings) {
    if (text.isEmpty()) {
        return {};
    }
    QString normalized = text;
    if (ignoreLineEndings) {
        normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    }
    return normalized.split(QLatin1Char('\n'));
}

} // namespace

QVector<DiffLine> DiffGenerator::generateDiff(const QString& oldText, const QString& newText, bool ignoreLineEndings) {
    const QStringList oldLines = splitLines(oldText, ignoreLineEndings);
    const QStringList newLines = splitLines(newText, ignoreLineEndings);

    const int n = oldLines.size();
    const int m = newLines.size();

    // Standard Longest Common Subsequence (LCS) matrix
    std::vector<std::vector<int>> lcs(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (oldLines[i] == newLines[j]) {
                lcs[i + 1][j + 1] = lcs[i][j] + 1;
            } else {
                lcs[i + 1][j + 1] = std::max(lcs[i + 1][j], lcs[i][j + 1]);
            }
        }
    }

    // Backtrack to build diff
    QVector<DiffLine> reversedDiff;
    int i = n;
    int j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && oldLines[i - 1] == newLines[j - 1]) {
            DiffLine line;
            line.type = DiffLineType::Context;
            line.oldLineNumber = i;
            line.newLineNumber = j;
            line.text = oldLines[i - 1];
            reversedDiff.append(line);
            --i;
            --j;
        } else if (j > 0 && (i == 0 || lcs[i][j - 1] >= lcs[i - 1][j])) {
            DiffLine line;
            line.type = DiffLineType::Addition;
            line.oldLineNumber = 0;
            line.newLineNumber = j;
            line.text = newLines[j - 1];
            reversedDiff.append(line);
            --j;
        } else if (i > 0 && (j == 0 || lcs[i][j - 1] < lcs[i - 1][j])) {
            DiffLine line;
            line.type = DiffLineType::Deletion;
            line.oldLineNumber = i;
            line.newLineNumber = 0;
            line.text = oldLines[i - 1];
            reversedDiff.append(line);
            --i;
        }
    }

    QVector<DiffLine> result;
    result.reserve(reversedDiff.size());
    for (int idx = reversedDiff.size() - 1; idx >= 0; --idx) {
        result.append(reversedDiff[idx]);
    }

    return result;
}

QString DiffGenerator::formatUnifiedDiff(const QVector<DiffLine>& diffLines) {
    QString output;
    for (const auto& line : diffLines) {
        output += line.prefixMarker();
        output += line.text;
        output += QLatin1Char('\n');
    }
    return output;
}

} // namespace CodeShelf
