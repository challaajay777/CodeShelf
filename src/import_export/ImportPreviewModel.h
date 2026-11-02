#pragma once

#include <QString>
#include <QVector>
#include "../core/Snippet.h"
#include "../core/Collection.h"
#include "../core/Tag.h"
#include "../core/Template.h"

namespace CodeShelf {

enum class ConflictResolution {
    Skip,
    KeepBothNewId,
    UpdateExisting
};

enum class ImportItemStatus {
    NewItem,
    DuplicateIdMatch,
    DuplicateContentMatch,
    Invalid
};

struct ImportSnippetItem {
    Snippet snippet;
    ImportItemStatus status{ImportItemStatus::NewItem};
    QString reason;
    QString existingSnippetId;
    ConflictResolution resolution{ConflictResolution::Skip};
};

struct ImportPreviewSummary {
    int totalInFile{0};
    int newCount{0};
    int duplicateIdCount{0};
    int duplicateContentCount{0};
    int invalidCount{0};
    QVector<ImportSnippetItem> snippetItems;
    QVector<Collection> collections;
    QVector<Template> templates;
    QStringList globalTags;
};

} // namespace CodeShelf
