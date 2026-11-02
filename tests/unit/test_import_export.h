#pragma once

#include <QObject>
#include <QTest>
#include "import_export/JsonBackupHandler.h"
#include "import_export/MarkdownExportHandler.h"
#include "repositories/SnippetRepository.h"
#include "repositories/CollectionRepository.h"
#include "repositories/TagRepository.h"
#include "repositories/TemplateRepository.h"
#include "database/Database.h"

namespace CodeShelf {

class TestImportExport : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMarkdownCodeFenceEscaping();
    void testJsonExportAndImportRoundTrip();
    void testImportInvalidJson();
    void testImportDuplicateIdConflict();
};

} // namespace CodeShelf
