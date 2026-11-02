#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/Revision.h"
#include "../database/Database.h"

namespace CodeShelf {

class RevisionRepository {
public:
    explicit RevisionRepository(Database& db = Database::instance());

    bool createRevision(const Revision& rev, QString& errorOut);
    std::optional<Revision> getById(const QString& id);
    QVector<Revision> getRevisionsForSnippet(const QString& snippetId);
    int getNextRevisionNumber(const QString& snippetId);

private:
    Database& m_db;
};

} // namespace CodeShelf
