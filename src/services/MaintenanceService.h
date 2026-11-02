#pragma once

#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "../database/Database.h"
#include "../repositories/SnippetRepository.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"

namespace CodeShelf {

struct DatabaseStats {
    qint64 fileSizeBytes{0};
    int activeSnippets{0};
    int trashSnippets{0};
    int totalCollections{0};
    int totalTags{0};
    int schemaVersion{1};
    bool fts5Enabled{false};
};

class MaintenanceService {
public:
    MaintenanceService(Database& db, SnippetRepository& snippetRepo,
                       CollectionRepository& colRepo, TagRepository& tagRepo);

    DatabaseStats getDatabaseStats();
    bool checkIntegrity(QStringList& issuesOut);
    bool vacuum(QString& errorOut);
    bool rebuildFts(QString& errorOut);

private:
    Database& m_db;
    SnippetRepository& m_snippetRepo;
    CollectionRepository& m_colRepo;
    TagRepository& m_tagRepo;
};

} // namespace CodeShelf
