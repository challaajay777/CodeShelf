#pragma once

#include <QString>
#include <QVector>
#include "ImportPreviewModel.h"
#include "../repositories/SnippetRepository.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"
#include "../repositories/TemplateRepository.h"

namespace CodeShelf {

class JsonBackupHandler {
public:
    JsonBackupHandler(SnippetRepository& snippetRepo, CollectionRepository& colRepo,
                      TagRepository& tagRepo, TemplateRepository& tmplRepo);

    // Export
    bool exportBackup(const QString& targetFilePath, const QVector<QString>& snippetIdsToExport,
                      bool includeRevisions, bool includeTemplates, QString& errorOut);

    // Import parsing & validation into preview
    bool parseAndPreview(const QString& sourceFilePath, ImportPreviewSummary& previewOut, QString& errorOut);

    // Apply previewed import with user-chosen conflict resolutions
    bool applyImport(ImportPreviewSummary& preview, ConflictResolution globalDefaultResolution, QString& errorOut);

private:
    SnippetRepository& m_snippetRepo;
    CollectionRepository& m_colRepo;
    TagRepository& m_tagRepo;
    TemplateRepository& m_tmplRepo;
};

} // namespace CodeShelf
