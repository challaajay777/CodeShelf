#include "MaintenanceService.h"
#include <QFileInfo>

namespace CodeShelf {

MaintenanceService::MaintenanceService(Database& db, SnippetRepository& snippetRepo,
                                       CollectionRepository& colRepo, TagRepository& tagRepo)
    : m_db(db)
    , m_snippetRepo(snippetRepo)
    , m_colRepo(colRepo)
    , m_tagRepo(tagRepo)
{
}

DatabaseStats MaintenanceService::getDatabaseStats() {
    DatabaseStats stats;
    QFileInfo fi(m_db.databasePath());
    if (fi.exists()) {
        stats.fileSizeBytes = fi.size();
    }
    stats.activeSnippets = m_snippetRepo.countActive();
    stats.trashSnippets = m_snippetRepo.countTrash();
    stats.totalCollections = m_colRepo.getAll().size();
    stats.totalTags = m_tagRepo.getAllTagsWithCounts().size();
    stats.schemaVersion = m_db.schemaVersion();
    stats.fts5Enabled = m_db.isFts5Available();
    return stats;
}

bool MaintenanceService::checkIntegrity(QStringList& issuesOut) {
    return m_db.checkIntegrity(issuesOut);
}

bool MaintenanceService::vacuum(QString& errorOut) {
    return m_db.vacuumDatabase(errorOut);
}

bool MaintenanceService::rebuildFts(QString& errorOut) {
    return m_db.rebuildFtsIndex(errorOut);
}

} // namespace CodeShelf
