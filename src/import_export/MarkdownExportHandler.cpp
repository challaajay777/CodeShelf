#include "MarkdownExportHandler.h"
#include <QFile>
#include <algorithm>

namespace CodeShelf {

QString MarkdownExportHandler::determineSafeCodeFence(const QString& body) {
    int maxBackticks = 0;
    int currentRun = 0;

    for (const QChar& ch : body) {
        if (ch == QLatin1Char('`')) {
            ++currentRun;
            if (currentRun > maxBackticks) {
                maxBackticks = currentRun;
            }
        } else {
            currentRun = 0;
        }
    }

    int fenceLen = std::max(3, maxBackticks + 1);
    return QString(fenceLen, QLatin1Char('`'));
}

QString MarkdownExportHandler::snippetToMarkdown(const Snippet& snippet) {
    QString md;

    // YAML Frontmatter
    md += QStringLiteral("---\n");
    md += QStringLiteral("title: \"%1\"\n").arg(QString(snippet.title()).replace(QLatin1Char('"'), QStringLiteral("\\\"")));
    md += QStringLiteral("language: \"%1\"\n").arg(snippet.language());
    md += QStringLiteral("type: \"%1\"\n").arg(snippetTypeToString(snippet.snippetType()));
    if (!snippet.tags().isEmpty()) {
        md += QStringLiteral("tags: [\"%1\"]\n").arg(snippet.tags().join(QStringLiteral("\", \"")));
    }
    if (!snippet.sourceUrl().isEmpty()) {
        md += QStringLiteral("source_url: \"%1\"\n").arg(snippet.sourceUrl());
    }
    md += QStringLiteral("created_at: \"%1\"\n").arg(snippet.createdAt().toString(Qt::ISODateWithMs));
    md += QStringLiteral("updated_at: \"%1\"\n").arg(snippet.updatedAt().toString(Qt::ISODateWithMs));
    md += QStringLiteral("---\n\n");

    // Title and description
    md += QStringLiteral("# %1\n\n").arg(snippet.title());
    if (!snippet.description().trimmed().isEmpty()) {
        md += snippet.description().trimmed();
        md += QStringLiteral("\n\n");
    }

    // Code block with safe fence
    const QString fence = determineSafeCodeFence(snippet.body());
    const QString langCode = snippet.language().toLower().replace(QLatin1Char(' '), QLatin1Char('-'));

    md += fence;
    md += langCode;
    md += QLatin1Char('\n');
    md += snippet.body();
    if (!snippet.body().endsWith(QLatin1Char('\n'))) {
        md += QLatin1Char('\n');
    }
    md += fence;
    md += QStringLiteral("\n\n");

    return md;
}

QString MarkdownExportHandler::exportMultipleToMarkdown(const QVector<Snippet>& snippets) {
    QString full;
    for (int i = 0; i < snippets.size(); ++i) {
        full += snippetToMarkdown(snippets[i]);
        if (i + 1 < snippets.size()) {
            full += QStringLiteral("\n---\n\n");
        }
    }
    return full;
}

bool MarkdownExportHandler::exportToFile(const QString& targetFilePath, const QString& markdownContent, QString& errorOut) {
    const QString tempPath = targetFilePath + QStringLiteral(".tmp");
    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorOut = QStringLiteral("Cannot open file for writing: %1").arg(targetFilePath);
        return false;
    }

    file.write(markdownContent.toUtf8());
    file.close();

    if (QFile::exists(targetFilePath)) {
        QFile::remove(targetFilePath);
    }
    if (!QFile::rename(tempPath, targetFilePath)) {
        errorOut = QStringLiteral("Failed to atomically finalize Markdown export file.");
        return false;
    }

    return true;
}

} // namespace CodeShelf
