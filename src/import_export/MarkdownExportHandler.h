#pragma once

#include <QString>
#include <QVector>
#include "../core/Snippet.h"

namespace CodeShelf {

class MarkdownExportHandler {
public:
    static QString snippetToMarkdown(const Snippet& snippet);
    static QString exportMultipleToMarkdown(const QVector<Snippet>& snippets);

    static bool exportToFile(const QString& targetFilePath, const QString& markdownContent, QString& errorOut);

    // Determines minimum fence length (e.g. ```, ````, `````) to prevent code-fence injection
    static QString determineSafeCodeFence(const QString& body);
};

} // namespace CodeShelf
